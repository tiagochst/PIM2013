#include "Vec3D.hpp"
#include "Image.h"
#include "SubImage.h"
#include "ImageBase.h"
#include "PixelTracker.h"
#include "TrackInfo.h"
#include "Config.h"
#include "Tools.h"
#include "Color.h"
#include "PPMImage.h"
#include "MathUtils.h"

#include <csignal>
#include <cmath>

void scream () {
    std::cout << "BOOM" << std::endl;
}

inline bool my_isnan (
    const float&    iValue
) {
    const unsigned char* cval = reinterpret_cast<const unsigned char*>(&iValue);

    static const float nqnan = -std::numeric_limits<float>::quiet_NaN();
    bool nqnantest = true;
    const unsigned char* cnan = reinterpret_cast<const unsigned char*>(&nqnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        nqnantest &= (cval[i] == cnan[i]);
    }
    static const float qnan = std::numeric_limits<float>::quiet_NaN();
    bool qnantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&qnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        qnantest &= (cval[i] == cnan[i]);
    }

    static const float nsnan = -std::numeric_limits<float>::signaling_NaN();
    bool nsnantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&nsnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        nsnantest &= (cval[i] == cnan[i]);
    }
    static const float snan = std::numeric_limits<float>::signaling_NaN();
    bool snantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&snan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        snantest &= (cval[i] == cnan[i]);
    }

    return (snantest || qnantest) || (nsnantest || nqnantest);
} 

/* Calculate  adjustment in the direction 
   of improved photometric-consistency */
inline float ComputeDp (
    const float&    e_1,
    const float&    e0,
    const float&    e1,
    const float&    p,
    const float&    q
) {
    float dp = q - p;
    if ( e_1 < e0 && e_1 < e1 ) {
        dp  -= 0.5f;
    } else if ( e1 < e_1 && e1 < e0 ) {
        dp  += 0.5f;
    } else if ( e0 < e_1 && e0 < e1 ) {
        dp  += ( 0.5f * ( e_1 - e1 ) / ( e_1 + e1 - 2.0f * e0 ) );
    }
    return dp; 
}

inline float ComputeUs (
    const int&              px,
    const int&              py,
    Image*                  iDepth,
    const Eigen::Matrix3f&  iNbh,
    const Eigen::Matrix3f&  iErr
) {
    float Us = 0.0f;
    float sigma = 100.0f;/* 1 mm? */

    CartesianCoordinate p ( px, py );
    for ( int x = -1; x <= 1; x++ ) {
        for ( int y = -1; y <= 1; y++ ) {
            CartesianCoordinate q ( px + x, py + y );

            /*Calculation made only with 8 neighboring pixels */
            if ( !x && !y ) continue;

            float delta = iDepth->GetGreyLvl (q) 
                        - iDepth->GetGreyLvl (p);

            float exponential = exp ( - ( delta * delta ) / ( sigma * sigma ) );
            float error = ( 1.0f - iErr ( y + 1, x + 1 ) ) / 2.0f;

            float disparity = iNbh ( y + 1, x + 1 );
            disparity = isinf ( disparity ) ? 0.0f : disparity;

            Us += disparity * exponential * ( 1.0f - error );
        }
    }
    return Us;
}
  
/* Calculate data-driven parameter wp*/
inline float ComputeWp (
    const float&    e_1, 
    const float&    e0,
    const float&    e1
) {
    float wp = 0.0f;
    if ( e_1 < e0 && e_1 <= e1 ) {
        wp = ( e0 - e_1 );
    } else if ( e1 <= e_1 && e1 < e0 ) {
        wp = ( e0 - e1 );
    } else if ( e0 < e_1 && e0 < e1 ) {
        wp = ( 0.5f * ( e_1 + e1 - 2.0f * e0 ) );
    }
    return wp;
}

/*  user-specified smoothness parameter ws */ 
inline float GetWs()
{
  /*Value based on the paper*/
  return 0.005f;
}

/* Calculate refinament adjustment */ 
inline Vec3Df ComputeDprime (
    const float& px,
    const float& py,
    const float& qx,
    const float& qy,
    Image*       iDepth,
    const Eigen::Matrix3f& iErr,
    const Eigen::Matrix3f& iNbhX,
    const Eigen::Matrix3f& iNbhY
) {
    float dpx = ComputeDp ( iErr ( 1, 0 ), iErr ( 1, 1 ) , iErr ( 1, 2 ), px, qx );
    float dpy = ComputeDp ( iErr ( 0, 1 ), iErr ( 1, 1 ) , iErr ( 2, 1 ), py, qy );

    //float ds = computeDsX(p);
    float usx = ComputeUs ( px, py, iDepth, iNbhX, iErr );
    float usy = ComputeUs ( px, py, iDepth, iNbhY, iErr );

    float wpx = ComputeWp ( iErr ( 1, 0 ), iErr ( 1, 1 ) , iErr ( 1, 2 ) );
    float wpy = ComputeWp ( iErr ( 0, 1 ), iErr ( 1, 1 ) , iErr ( 2, 1 ) );

    const float ws = GetWs ();

    /* return d'*/
    Vec3Df dprime;
    dprime[0]  = ( wpx * dpx + ws * usx ) / ( wpx + ws );
    dprime[1]  = ( wpy * dpy + ws * usy ) / ( wpy + ws );

    if (
            my_isnan ( dprime[0] )
        ||  my_isnan ( dprime[1] )
    ) {
        scream ();
    }
    return dprime;
}


PixelTracker::PixelTracker (
    const unsigned int&         iReferenceId
)   :   m_referenceId ( iReferenceId ),
        m_refImage ( (Image*)0x0 ),
        m_winHeight ( 1 ),
        m_winWidth ( 1 ),
        m_nbhHeight ( 9 ),
        m_nbhWidth ( 9 ),
        m_rejectionTreshold ( 0.f ),
        m_forwardTrack ( (TrackInfo*)0x0 ),
        m_backwardsTrack ( (TrackInfo*)0x0 ),
        m_disparityMapX (),
        m_disparityMapY (),
        m_depthMap (), 
        m_refId ( 0 ),
        m_tarId ( 0 ),
        m_refImgPyr (),
        m_refDepPyr (),
        m_tarImgPyr (),
        m_tarDepPyr (),
        m_dispX (),
        m_dispY ()
{
    m_refImgPyr.SetSamplingFactor ( 2 );
    m_refDepPyr.SetSamplingFactor ( 2 );
    m_tarImgPyr.SetSamplingFactor ( 2 );
    m_tarDepPyr.SetSamplingFactor ( 2 );
    m_dispX.SetSamplingFactor ( 2 );
    m_dispY.SetSamplingFactor ( 2 );
}

PixelTracker::~PixelTracker () {
    if ( m_refImage ) {
        delete m_refImage;
        m_refImage = (Image*)0x0;
    }
}

void PixelTracker::Reset ()
{
    m_disparityMapX.fill (
        std::numeric_limits<float>::infinity ()        
    );
    m_disparityMapY.fill (
        std::numeric_limits<float>::infinity ()        
    );

    if ( m_forwardTrack ) {
        delete m_forwardTrack;
        m_forwardTrack = (TrackInfo*)0x0;
    }
    if ( m_backwardsTrack ) {
        delete m_backwardsTrack;
        m_backwardsTrack = (TrackInfo*)0x0;
    }
}

void PixelTracker::SetReference (
    const unsigned int&     iReferenceId,
    const Image*            iReferenceImage,
    const Image*            iReferenceDepth
) {
    m_refId = iReferenceId;
    m_refImgPyr.Assign (
        iReferenceImage
    );
    m_refDepPyr.Assign (
        iReferenceDepth
    );
}
void PixelTracker::SetTarget (
    const unsigned int&     iTargetId,
    const Image*            iTargetImage,
    const Image*            iTargetDepth
) {
    m_tarId = iTargetId;
    m_tarImgPyr.Assign (
        iTargetImage
    );
    m_tarDepPyr.Assign (
        iTargetDepth
    );
}

void PixelTracker::Track ()
{
    Image* refImage = m_refImgPyr.Bottom ();
    Image* blankImage = new Image (
        refImage->GetWidth (),
        refImage->GetHeight (),
        refImage->GetMaxGreyLevel ()        
    );
    m_dispX.Assign ( blankImage );
    m_dispY.Assign ( blankImage );
    delete blankImage;
    blankImage = (Image*)0x0;

    // Processes pyramids from top to bottom. The coarsest level is processed 
    // once independently so we can estimate extremal motions on each direction.
    Image* ref  = m_refImgPyr.Top ();
    Image* tar  = m_tarImgPyr.Top ();
    Image* dX   = m_dispX.Top ();
    Image* dY   = m_dispY.Top ();

    int mx=INT_MIN, my=INT_MIN, mx_=INT_MAX, my_=INT_MAX;
    // Track a fixed set of features from ref to tar.
    for ( int i = 3; i < ref->GetWidth () - 3; i += 15 ) {
        for ( int j = 3; j < ref->GetHeight () - 3; j += 15 ) {
            Image nbh;

            // Tracks a patch of size 3x3 on target image.
            ref->GetSubImage (
                i - 3, j - 3,
                7, 7,
                nbh
            );
            CartesianCoordinate best;
            tar->TemplateMatch ( nbh, best );

            std::cout << "(i,j) = " << i    << " " << j << std::endl;
            std::cout << "best = "  << best.x << " " << best.y << std::endl;
            std::cout << "delta = " << best.x - i << " " << best.y - j << std::endl;
 
            // Extremal movement estimation.
            mx  = max ( mx, best.x - i ); 
            my  = max ( my, best.y - j ); 
            mx_ = min ( mx_, best.x - i ); 
            my_ = min ( my_, best.y - j ); 
        }
    }
    // Initial window size calculation.
    //m_winHeight = max ( my - my_, 3 );
    //m_winWidth  = max ( mx - mx_, 3 );
    m_winHeight = 9;
    m_winWidth  = 9;
    std::cout << "Initial window size: " << m_winWidth << "x" << m_winHeight <<std::endl;
    std::cout << "mx+ = " << mx << std::endl;
    std::cout << "mx- = " << mx_ << std::endl;
    std::cout << "my+ = " << my << std::endl;
    std::cout << "my- = " << my_ << std::endl;

    // Process the rest of the pyramid.
    for ( int i = m_refImgPyr.GetNumLevels () - 1; i >= 0; i-- ) {
        PyramidTrack ( i );

        // Every level but the coarsest uses a 3x3 search window for pixel tracking.
        if ( i == ( m_refImgPyr.GetNumLevels () - 1 ) ) {
            m_winWidth = m_winHeight = 3;
        }
    }
}

void PixelTracker::PyramidTrack (
    const unsigned int&     iLevel
) {
    PyramidMatch        ( iLevel );
    PyramidSmooth       ( iLevel );
    PyramidOrdering     ( iLevel );
    PyramidUniqueness   ( iLevel );
    PyramidRematch      ( iLevel );
    PyramidUniqueness   ( iLevel );
    const unsigned int maxIter = 80u;
    const unsigned int minIter = 20u;
    const unsigned int nIter = maxIter - ( (maxIter-minIter) * (iLevel+1) ) / m_refImgPyr.GetNumLevels ();
    //const unsigned int nIter = 1;
    //for ( unsigned int i = 0; i < nIter; i++ ) {
    //    PyramidRefine       ( iLevel );
    //}
    //PyramidUniqueness   ( iLevel );
}

void PixelTracker::PyramidMatch (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );
    
    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            int deltaX = 0;
            int deltaY = 0;
            if ( iLevel < m_refImgPyr.GetNumLevels () - 1 ) {
                float fdX = m_dispX[iLevel + 1]->GetNormed ( j/2, i/2 ) * 2;
                float fdY = m_dispY[iLevel + 1]->GetNormed ( j/2, i/2 ) * 2;
                if ( !isinf(fdX) && !isinf(fdY) ) {
                    deltaX = nearbyint(fdX); 
                    deltaY = nearbyint(fdY); 
                }
            }

            Image nbh;
            ref->GetSubImage (
                i - m_nbhWidth / 2, j - m_nbhHeight / 2,
                m_nbhWidth, m_nbhHeight,
                nbh
            );

            CartesianCoordinate best;
            tar->TemplateMatch (
                nbh,
                i - m_winWidth / 2  + deltaX,
                j - m_winHeight / 2 + deltaY,
                m_winWidth,
                m_winHeight,
                best
            ); 
            //std::cout << "(i,j) = " << i    << " " << j << std::endl;
            //std::cout << "best = "  << best.x << " " << best.y << std::endl;
            //std::cout << "delta = " << best.x - i << " " << best.y - j << std::endl;

            newDX->SetNormed ( j, i, best.x - i ) ;
            newDY->SetNormed ( j, i, best.y - j ) ;
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidRematch (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );
    
    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            Image nbh;

            int deltaX = 0;
            int deltaY = 0;
            float fdX = dX->GetNormed (     j, i + 1 );
            float fdY = dY->GetNormed ( j + 1, i     );
            if ( !isinf(fdX) ) {
                deltaX = nearbyint(fdX);
            }
            if ( !isinf(fdY) ) {
                deltaY = nearbyint(fdY); 
            }

            ref->GetSubImage (
                i - m_nbhWidth / 2, j - m_nbhHeight / 2,
                m_nbhWidth, m_nbhHeight,
                nbh
            );

            CartesianCoordinate best;
            tar->TemplateMatch (
                nbh,
                i - m_winWidth / 2  + deltaX,
                j - m_winHeight / 2 + deltaY,
                m_winWidth,
                m_winHeight,
                best
            ); 

            newDX->SetNormed ( j, i, best.x - i ) ;
            newDY->SetNormed ( j, i, best.y - j ) ;
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidSmooth (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );

    static const float inf = std::numeric_limits<float>::infinity ();
    const int width = ref->GetWidth ();
    const int height = ref->GetHeight ();

    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < width; i++ ) {
        for ( int j = 0; j < height; j++ ) {
            Eigen::Matrix3f ndX;
            Eigen::Matrix3f ndY;
            ndX.fill ( inf );
            ndY.fill ( inf );
            unsigned int count = 0;
            unsigned int total = 0;

            const float& dispX = dX->GetNormed ( j, i );
            const float& dispY = dY->GetNormed ( j, i );
            for ( int ii = i - 1; ii < i + 1; ii++ ) {
                for ( int jj = j - 1; jj < j + 1; jj++ ) {
                    if (ii != i && jj != j) continue;
                    unsigned int idX = ii - i + 1;
                    unsigned int idY = jj - j + 1;
                    if (
                            ( jj >= 0 && ii >= 0 )
                        &&  ( jj < height && ii < width )
                    ) {
                        float& a = ndX ( idY, idX ) = dX->GetNormed ( jj, ii ) - dispX;
                        float& b = ndY ( idY, idX ) = dY->GetNormed ( jj, ii ) - dispY;

                        if (
                                fabs (a) <= 1.0f
                            &&  fabs (b) <= 1.0f
                        ) {
                            count++;
                        }
                        total++;
                    }
                }
            }
            if (
                    !total
                ||  ( (float)count / (float)total ) < 0.5f 
            ) {
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
            } else {
                newDX->SetNormed ( j, i, dispX ) ;
                newDY->SetNormed ( j, i, dispY ) ;
            }
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidOrdering (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );

    static const float inf = std::numeric_limits<float>::infinity ();
    const int width = ref->GetWidth ();
    const int height = ref->GetHeight ();

    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < width; i++ ) {
        for ( int j = 0; j < height; j++ ) {
            const float& dispX  = dX->GetNormed (     j, i     );
            const float& dispY  = dY->GetNormed (     j, i     );
            const float& dispXX = dX->GetNormed (     j, i + 1 ) - dispX;
            const float& dispYX = dY->GetNormed (     j, i + 1 ) - dispY;
            const float& dispXY = dX->GetNormed ( j + 1, i     ) - dispX;
            const float& dispYY = dY->GetNormed ( j + 1, i     ) - dispY;

            if (
                    isinf ( dispXX )  || isinf ( dispXY )
                ||  isinf ( dispYX )  || isinf ( dispYY )
                ||  ( dispXX > 1.0f ) || ( dispXY > 1.0f )
                ||  ( dispYX > 1.0f ) || ( dispYY > 1.0f )
            ) {
                newDX->SetNormed ( j, i, inf );
                newDY->SetNormed ( j, i, inf );
            } else {

                newDX->SetNormed ( j, i, dispX );
                newDY->SetNormed ( j, i, dispY );
            }
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidUniqueness (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );
    
    static const float inf = std::numeric_limits<float>::infinity();

    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            Image nbh;

            int deltaX = 0;
            int deltaY = 0;
            const float& dispX = dX->GetNormed ( j, i );
            const float& dispY = dY->GetNormed ( j, i );
            if ( !isinf(dispX) && !isinf(dispY) ) {
                deltaX = nearbyint(dispX); 
                deltaY = nearbyint(dispY); 
            } else {
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
                continue;
            }

            tar->GetSubImage (
                i - m_nbhWidth / 2 + deltaX, j - m_nbhHeight / 2 + deltaY,
                m_nbhWidth, m_nbhHeight,
                nbh
            );

            CartesianCoordinate best;
            ref->TemplateMatch (
                nbh,
                i - m_winWidth / 2,
                j - m_winHeight / 2,
                m_winWidth,
                m_winHeight,
                best
            ); 

            if (
                    ( best.x - i ) <= 1.0f
                &&  ( best.y - j ) <= 1.0f
            ) {
                newDX->SetNormed ( j, i, dispX ) ;
                newDY->SetNormed ( j, i, dispY ) ;
            } else {
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
            }
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidRefine (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* dep      = m_refDepPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );
    
    static const float inf = std::numeric_limits<float>::infinity();

    //#pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            Image nbh;

            const float& dispX = dX->GetNormed ( j, i );
            const float& dispY = dY->GetNormed ( j, i );
            if ( isinf ( dispX ) || isinf ( dispY ) ) {
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
                continue;
            }
            //std::cout << "cheguei aqui" << std::endl;

            Eigen::Matrix3f nbhX;
            Eigen::Matrix3f nbhY;
            Eigen::Matrix3f err;
            nbhX.fill ( 0 );
            nbhY.fill ( 0 );
            err.fill ( 0 );

            CartesianCoordinate p ( i, j );
            for ( int ii = -1; i < 1; i++ ) {
                for ( int jj = -1; jj < 1; jj++ ) {
                    CartesianCoordinate q (
                        nearbyint ( i + dispX ) + ii,
                        nearbyint ( j + dispY ) + jj
                    );
                    float error = ( 1.0f - ref->PixelCorrelation ( *tar, p , q, 3, 3 ) ) / 2.0f;

                    nbhX ( jj + 1, ii + 1 ) = dX->GetNormed ( j + jj, i + ii );
                    nbhY ( jj + 1, ii + 1 ) = dY->GetNormed ( j + jj, i + ii );
                    err  ( jj + 1, ii + 1 ) = error;
                }
            }

            Vec3Df newDisp = ComputeDprime (
                i, j,
                nearbyint ( i + dispX ), nearbyint ( j + dispY ),
                m_refDepPyr[iLevel],
                err,
                nbhX,
                nbhY
            );

            newDX->SetNormed ( j, i, newDisp[0] ) ;
            newDY->SetNormed ( j, i, newDisp[1] ) ;
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::SetUp (
    const unsigned int&     iWindowWidth,
    const unsigned int&     iWindowHeight,
    const unsigned int&     iNeighbourhoodWidth,
    const unsigned int&     iNeighbourhoodHeight,
    const float&            iRejectionTreshold
) {
    m_winWidth          =   iWindowWidth;
    m_winHeight         =   iWindowHeight;
    m_nbhWidth          =   iNeighbourhoodWidth;
    m_nbhHeight         =   iNeighbourhoodHeight;
    m_rejectionTreshold =   iRejectionTreshold;

    if ( m_refImage ) {
        delete m_refImage;
    }
    m_refImage = new Image (
        Config::OutputPath () + "CapturedFrames/image_" + Int2Str ( m_referenceId ) + ".pgm"
    );

    if ( m_depthMap ) {
        delete m_depthMap;
    }
    m_depthMap = new Image (
        Config::OutputPath () + "CapturedFrames/depth_" + Int2Str ( m_referenceId ) + ".pgm"
    );

    m_disparityMapX.resize (
        m_refImage->GetHeight (),
        m_refImage->GetWidth ()
    );
    m_disparityMapY.resize (
        m_refImage->GetHeight (),
        m_refImage->GetWidth ()
    );
}

void PixelTracker::SetRejectionTreshold (
    const float&            iRejectionTreshold
) {
    m_rejectionTreshold = iRejectionTreshold;
}

void PixelTracker::TrackWithRegulation (
    const unsigned int&     iTargetFrameId
) {
    Image target ( Config::OutputPath () + "CapturedFrames/image_" + Int2Str(iTargetFrameId) + ".pgm" );
    return TrackWithRegulation ( iTargetFrameId, &target );
}
void PixelTracker::Track (
    const unsigned int&     iTargetFrameId
) {
    Image target ( Config::OutputPath () + "CapturedFrames/image_" + Int2Str(iTargetFrameId) + ".pgm" );
    return Track ( iTargetFrameId, &target );
}

void PixelTracker::TrackWithRegulation (
    const unsigned int&     iTargetFrameId,
    const Image*            iTarget
) {
    if (
            m_refImage->GetHeight() != iTarget->GetHeight()
        ||  m_refImage->GetWidth()  != iTarget->GetWidth()
    ) {
        throw IncompatibleImages();
    }
    Reset ();
    m_forwardTrack = new TrackInfo ( m_referenceId, iTargetFrameId );
    m_forwardTrack->SetDimensions (
        m_refImage->GetWidth (),
        m_refImage->GetHeight ()
    );

    CartesianCoordinate nCenter ( m_nbhWidth / 2, m_nbhHeight / 2 ); 
    CartesianCoordinate wCenter ( m_winWidth / 2, m_winHeight / 2 );

    SubImage neighbourhoodL;
    SubImage neighbourhoodR;

    #pragma omp parallel for
    for ( int x = 0; x < m_refImage->GetWidth(); x++ ) {
        #pragma omp parallel for private (neighbourhoodR, neighbourhoodL) 
        for ( int y = 0; y < m_refImage->GetHeight(); y++ ) {
            CartesianCoordinate bestMatchL;
            CartesianCoordinate bestMatchR;
            Image correlationMapL;
            Image correlationMapR; 

            m_refImage->GetSubImage (
                x - nCenter.x,
                y - nCenter.y,
                m_nbhWidth,
                m_nbhHeight,
                neighbourhoodL
            );
            Rectangle wRegionL (
                x - wCenter.x,
                y - wCenter.y,
                m_winWidth,
                m_winHeight
            );
            m_refImage->GetSubImage (
                x - nCenter.x + 1,
                y - nCenter.y,
                m_nbhWidth,
                m_nbhHeight,
                neighbourhoodR
            );
            Rectangle wRegionR (
                x - wCenter.x + 1,
                y - wCenter.y,
                m_winWidth,
                m_winHeight
            );

            iTarget->TemplateMatch (
                neighbourhoodL,
                wRegionL,
                bestMatchL,
                &correlationMapL
            );
            iTarget->TemplateMatch (
                neighbourhoodR,
                wRegionR,
                bestMatchR,
                &correlationMapR
            );
            
            MatchDescriptor md;
            static const float inf = std::numeric_limits<float>::infinity();
            float bestScore = -inf;
            float bestR = -inf;
            float bestL = -inf;
            for ( int xx = 0; xx < correlationMapL.GetWidth (); xx++ ) {
                for ( int yy = 0; yy < correlationMapL.GetHeight (); yy++ ) {
                    float leftVal  = correlationMapL.GetNormed ( yy, xx );
                    float rightVal = correlationMapR.GetNormed ( yy, xx );
    
                    float score = sqrt (
                             leftVal * leftVal
                        +   rightVal * rightVal
                    ); 
                    if (
                        //    leftVal >= bestL
                        //&& rightVal >  bestR
                        score >= bestScore
                    ) {
                        bestMatchL.x = xx + wRegionL.X();
                        bestMatchL.y = yy + wRegionL.Y();

                        //bestL = leftVal;
                        //bestR = rightVal;
                        //bestScore = bestL;
                        bestScore = score;
                    }
                }
            }
            if (
                bestScore >= m_rejectionTreshold
            ) {
                md.m_score  = (bestScore - m_rejectionTreshold ) / (1.0f - m_rejectionTreshold);
                md.m_xCoord = bestMatchL.x;
                md.m_yCoord = bestMatchL.y;
            } else {
                md.m_score  = -1;
                md.m_xCoord = 0u;
                md.m_yCoord = 0u;
            }
            m_forwardTrack->SetMatch ( x, y, md );
        }
    }
}
void PixelTracker::Track (
    const unsigned int&     iTargetFrameId,
    Image*                  iTarget
) {
    if (
            m_refImage->GetHeight() != iTarget->GetHeight()
        ||  m_refImage->GetWidth()  != iTarget->GetWidth()
    ) {
        throw IncompatibleImages();
    }

    Reset ();
    m_forwardTrack = new TrackInfo ( m_referenceId, iTargetFrameId );
    m_forwardTrack->SetDimensions (
        m_refImage->GetWidth (),
        m_refImage->GetHeight ()
    );
    m_backwardsTrack = new TrackInfo ( iTargetFrameId, m_referenceId );
    m_backwardsTrack->SetDimensions (
        m_refImage->GetWidth (),
        m_refImage->GetHeight ()
    );
    
    unsigned int unmatchedPixels    = m_refImage->GetWidth ()
                                    * m_refImage->GetHeight ();
    std::cout << unmatchedPixels << " pixels to track." << std::endl;
    unsigned int nIter = 0;
    while (
            ( nIter++ < 10u )
        &&  ( unmatchedPixels > 0 )
    ) {
        unmatchedPixels = Match ( unmatchedPixels, iTarget );
        std::cout << unmatchedPixels << " left. Rematching..." << std::endl;
    }
}

unsigned int PixelTracker::Match (
    const unsigned int&     iUnmatchedPixels,
    Image*                  iTarget
) {
    static const float inf = std::numeric_limits<float>::infinity();
    unsigned int unmatchedPixels = iUnmatchedPixels;

    CartesianCoordinate nCenter ( m_nbhWidth / 2, m_nbhHeight / 2 ); 
    CartesianCoordinate wCenter ( m_winWidth / 2, m_winHeight / 2 );

    SubImage neighbourhood;

    const unsigned int width  = m_refImage->GetWidth (); 
    const unsigned int height = m_refImage->GetHeight ();
    const unsigned int totalPixels = width * height;
    Eigen::MatrixXf disparityMapX;
    Eigen::MatrixXf disparityMapY;

    disparityMapX.resize ( height, width );
    disparityMapY.resize ( height, width );

    #pragma omp parallel for
    for ( int x = 0; x < width; x++ ) {
        #pragma omp parallel for private (neighbourhood) 
        for ( int y = 0; y < height; y++ ) {
            if (
                    !isinf ( m_disparityMapX ( y, x ) ) 
                ||  !isinf ( m_disparityMapY ( y, x ) )
            ) {
                continue;
            } 

            MatchDescriptor fmd;
            MatchDescriptor bmd;
            CartesianCoordinate bestMatch;
            CartesianCoordinate dummy;
            Image correlationMap;

            float score         = -inf;
            float rightScore    = -inf;
            float leftScore     = -inf;
            float backScore     = -inf;

            Eigen::Matrix3f ndX;
            Eigen::Matrix3f ndY;
            ndX.fill ( inf );
            ndY.fill ( inf );

            float minDispX =  inf;
            float minDispY =  inf;
            float maxDispX = -inf;
            float maxDispY = -inf;
            #pragma omp critical
            {
                for ( int xx = x - 1; xx < x + 1; xx++ ) {
                    for ( int yy = y - 1; yy < y + 1; yy++ ) {
                        if (xx != x && yy != y) continue;
                        unsigned int idX = xx - x + 1;
                        unsigned int idY = yy - y + 1;
                        if (
                                ( yy >= 0 && xx >= 0 )
                            &&  ( yy < height && xx < width )
                        ) {
                            float& a = ndX ( idY, idX ) = m_disparityMapX ( yy, xx );
                            float& b = ndY ( idY, idX ) = m_disparityMapY ( yy, xx );

                            minDispX = fmin ( a, minDispX );
                            minDispY = fmin ( b, minDispY );
                            maxDispX = fmax ( a, maxDispX );
                            maxDispY = fmax ( b, maxDispY );
                        }
                    }
                }
            }
            minDispX = ( isinf ( minDispX ) ) ? 0 : minDispX;
            minDispY = ( isinf ( minDispY ) ) ? 0 : minDispY;
            maxDispX = ( isinf ( maxDispX ) ) ? 0 : maxDispX;
            maxDispY = ( isinf ( maxDispY ) ) ? 0 : maxDispY;

            //#pragma omp critical
            //{
            //    if (minDispX) std::cout << "mX " << minDispX << " ";
            //    if (minDispY) std::cout << "mY " << minDispY << " ";
            //    if (maxDispX) std::cout << "MX " << maxDispX << " ";
            //    if (maxDispY) std::cout << "MY " << maxDispY << " ";
            //}
            m_refImage->GetSubImage (
                x - m_nbhWidth / 2,     // X
                y - m_nbhHeight / 2,    // Y
                m_nbhWidth,             // Width
                m_nbhHeight,            // Height
                neighbourhood
            );
            fmd.m_score  = iTarget->TemplateMatch (
                neighbourhood,
                x - m_winWidth / 2   + minDispX,                // X
                y - m_winHeight / 2  + minDispY,                // Y
                m_winWidth           + (maxDispX - minDispX),   // Width
                m_winHeight          + (maxDispY - minDispY),   // Height
                bestMatch,
                &correlationMap
            );
            fmd.m_xCoord = bestMatch.x;
            fmd.m_yCoord = bestMatch.y;
            m_forwardTrack->SetMatch ( x, y, fmd );

            iTarget->GetSubImage (
                bestMatch.x - m_nbhWidth / 2,   // X
                bestMatch.y - m_nbhHeight / 2,  // Y
                m_nbhWidth,                     // Width
                m_nbhHeight,                    // Height
                neighbourhood
            );
            bmd.m_score  = m_refImage->TemplateMatch (
                neighbourhood,
                bestMatch.x - m_winWidth / 2    ,//+ minDispX,                // X
                bestMatch.y - m_winHeight / 2   ,//+ minDispY,                // Y
                m_winWidth                      ,//+ (maxDispX - minDispX),   // Width
                m_winHeight                     ,//+ (maxDispY - minDispY),   // Height
                bestMatch,
                &correlationMap
            );
            bmd.m_xCoord = bestMatch.x;
            bmd.m_yCoord = bestMatch.y;
            m_backwardsTrack->SetMatch ( x, y, bmd );

            #pragma omp critical
            {
                disparityMapX ( y, x ) = fmd.m_xCoord - x;
                disparityMapY ( y, x ) = fmd.m_yCoord - y;

                //if ( abs(m_disparityMapX ( y, x )) > m_winWidth / 2 )
                //    std::cerr << "Error DispX " << fmd.m_xCoord << " " << x << " " << y << std::endl;
                //if ( abs(m_disparityMapY ( y, x )) > m_winHeight / 2 )
                //    std::cerr << "Error DispY " << fmd.m_yCoord << " " << x << " " << y << std::endl;
            }
        }
    }
    #pragma omp parallel for
    for ( unsigned int x = 0; x < width; x++ ) {
        #pragma omp parallel for
        for ( unsigned int y = 0; y < height; y++ ) {
            MatchDescriptor& fmd = m_forwardTrack->GetMatchData ( x, y );
            MatchDescriptor& bmd = m_backwardsTrack->GetMatchData ( x, y );

            if (
                fmd.m_matched
            ) {
                continue;
            } 

            bool match = true;

            Eigen::Matrix3f ndX;
            Eigen::Matrix3f ndY;
            ndX.fill ( inf );
            ndY.fill ( inf );
            unsigned int count = 0;
            unsigned int total = 0;
            #pragma omp critical
            {
                const float& dX = disparityMapX ( y, x );
                const float& dY = disparityMapY ( y, x );
                for ( int xx = x - 1; xx < x + 1; xx++ ) {
                    for ( int yy = y - 1; yy < y + 1; yy++ ) {
                        if (xx != x && yy != y) continue;
                        unsigned int idX = xx - x + 1;
                        unsigned int idY = yy - y + 1;
                        if (
                                ( yy >= 0 && xx >= 0 )
                            &&  ( yy < height && xx < width )
                        ) {
                            float& a = ndX ( idY, idX ) = disparityMapX ( yy, xx ) - dX;
                            float& b = ndY ( idY, idX ) = disparityMapY ( yy, xx ) - dY;

                            if ( a <= 1.0f && b <= 1.0f ) {
                                count++;
                            }
                            total++;
                        }
                    }
                }
            }
            if ( total ) {
                match &= ( ( (float)count / (float)total ) >= 0.5f );
            }
            
            Vec3Df fme = Vec3Df (
                (float)fmd.m_xCoord - x,
                (float)fmd.m_yCoord - y,
                0.0f
            );
            Vec3Df bme = Vec3Df (
                (float)bmd.m_xCoord - fmd.m_xCoord,
                (float)bmd.m_yCoord - fmd.m_yCoord,
                0.0f
            );

            float& rDispX = ndX ( 0, 1 );
            float& rDispY = ndY ( 0, 1 );
            if (
                    !isinf ( rDispX )
                &&  !isinf ( rDispY )
            ) {
                match &= abs( fme[0] - rDispX ) <= 1.0f;
                match &= abs( fme[1] - rDispY ) <= 1.0f;
            }
            Vec3Df me = fme + bme;
            match &= ( me[0] <= 1.0f );
            match &= ( me[1] <= 1.0f );

            #pragma omp critical
            {
                if ( match ) {
                    unmatchedPixels--;
                    fmd.m_matched = true;
                    bmd.m_matched = true;
                } else {
                    disparityMapX ( y, x ) = 0;
                    disparityMapY ( y, x ) = 0;
                }
            }
        }
    }

    m_disparityMapX = disparityMapX;
    m_disparityMapY = disparityMapY;

    return unmatchedPixels;
}

void printColorChart (
    const int& width,
    const int& height,
    PPMImage& colorChart
) {
    const float maxDispX = width  * 0.5f;   
    const float maxDispY = height * 0.5f;
    const float maxDisp = sqrt ( maxDispX * maxDispX + maxDispY * maxDispY );

    colorChart.ResetDimensions ( width, height );
    colorChart.SetMaxValue ( 255u );
    Color c;
    Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    for ( unsigned int x = 0; x < width; x++ ) {
        for ( unsigned int y = 0; y < height; y++ ) {
            const float dX = x - maxDispX; 
            const float dY = y - maxDispY;

            Vec3Df disp ( dX, dY, 0.0f );
            const float value = disp.normalize () / maxDisp;

            const float hue = 180.0f * acos ( Vec3Df::dotProduct ( disp, redDir ) ) / M_PI; 
            if ( disp [1] < 0 ) {
                c.FromHSV ( 360.0f-hue, 1.0f, value ); 
            } else {
                c.FromHSV ( hue, 1.0f, value ); 
            }
            //std::cout << c.Red () << " " << c.Green () << " " << c.Blue () << std::endl;

            colorChart.SetChannelValue ( y, x,   RED, 255.f * c.Red () );
            colorChart.SetChannelValue ( y, x, GREEN, 255.f * c.Green () );
            colorChart.SetChannelValue ( y, x,  BLUE, 255.f * c.Blue () );
        }
    }
    colorChart.WriteToFile ( "colorChart.ppm", PIXMAP | BINARY );
}


void PixelTracker::Export (
    const std::string&      iFilename
) const {
    if ( m_forwardTrack ) {
        m_forwardTrack->CreateOutputImage ( iFilename + "forward" );
    }
    if ( m_backwardsTrack ) {
        m_backwardsTrack->CreateOutputImage ( iFilename + "backwards" );
    }

    const float inf = std::numeric_limits<float>::infinity();

    PPMImage disparityMap;
    //disparityMap.SetMaxValue ( 255u );
    //disparityMap.ResetDimensions (
    //    m_refImage->GetWidth (),
    //    m_refImage->GetHeight ()        
    //);
    //PPMImage colorChart;
    //printColorChart ( 30, 30, colorChart);

    //const float maxDispX = m_winWidth  * 0.5f;
    //const float maxDispY = m_winHeight * 0.5f;
    //float maxDispX = -inf;
    //float maxDispY = -inf;
    //for ( unsigned int x = 0; x < m_refImage->GetWidth (); x++ ) {
    //    for ( unsigned int y = 0; y < m_refImage->GetHeight (); y++ ) {
    //        if ( !isinf(m_disparityMapX ( y, x ) ) ) 
    //            maxDispX = fmax ( fabs ( m_disparityMapX ( y, x ) ), maxDispX );
    //        if ( !isinf(m_disparityMapY ( y, x ) ) ) 
    //            maxDispY = fmax ( fabs ( m_disparityMapY ( y, x ) ), maxDispY );
    //    }
    //}
    //std::cout << maxDispX << " " << maxDispY << std::endl;

    //float maxDisp = sqrt ( maxDispX * maxDispX + maxDispY * maxDispY );

    //Color c;
    //Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    //for ( unsigned int x = 0; x < m_refImage->GetWidth (); x++ ) {
    //    for ( unsigned int y = 0; y < m_refImage->GetHeight (); y++ ) {
    //        const float dX = m_disparityMapX ( y, x );
    //        const float dY = m_disparityMapY ( y, x );

    //        if ( isinf(dX) || isinf(dY) ) {
    //            disparityMap.SetChannelValue ( y, x, GREY, 255u );
    //            continue;
    //        }

    //        Vec3Df disp ( dX, dY, 0.0f );
    //        //std::cout << dX << " " << dY << " ";
    //        const float value = disp.normalize () / maxDisp;
    //        //std::cout << value << std::endl;

    //        const float hue = 180.0f * acos ( Vec3Df::dotProduct ( disp, redDir ) ) / M_PI; 
    //        if ( disp [1] < 0 ) {
    //            c.FromHSV ( 360.0f-hue, 1.0f, value ); 
    //        } else {
    //            c.FromHSV ( hue, 1.0f, value ); 
    //        }

    //        disparityMap.SetChannelValue ( y, x,   RED, 255.f * c.Red () );
    //        disparityMap.SetChannelValue ( y, x, GREEN, 255.f * c.Green () );
    //        disparityMap.SetChannelValue ( y, x,  BLUE, 255.f * c.Blue () );
    //    }
    //}
    //disparityMap.WriteToFile ( iFilename, PIXMAP | ASCII );

    Color c;
    Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    for ( int i = m_dispX.GetNumLevels () - 1; i >= 0; i-- ) {
        const Image* dispX = m_dispX[i];
        const Image* dispY = m_dispY[i];
        float maxDispX = -inf;
        float maxDispY = -inf;
        for ( unsigned int x = 0; x < dispX->GetWidth (); x++ ) {
            for ( unsigned int y = 0; y < dispX->GetHeight (); y++ ) {
                const float dX = dispX->GetNormed ( y, x );
                const float dY = dispY->GetNormed ( y, x );
                if ( !isinf(dX) && !isinf(dY) ) {
                    maxDispX = std::max ( (float)abs ( dX ), maxDispX );
                    maxDispY = std::max ( (float)abs ( dY ), maxDispY );
                }
            }
        }
        float maxDisp = sqrt ( maxDispX*maxDispX + maxDispY*maxDispY );

        disparityMap.SetMaxValue ( 255u );
        disparityMap.ResetDimensions (
            dispX->GetWidth (),
            dispX->GetHeight ()        
        );
        for ( unsigned int x = 0; x < dispX->GetWidth (); x++ ) {
            for ( unsigned int y = 0; y < dispX->GetHeight (); y++ ) {
                const float dX = dispX->GetNormed ( y, x );
                const float dY = dispY->GetNormed ( y, x );

                if ( isinf(dX) || isinf(dY) ) {
                    disparityMap.SetChannelValue ( y, x, GREY, 255u );
                    continue;
                }

                Vec3Df disp ( dX, dY, 0.0f );
                //std::cout << dX << " " << dY << " ";
                const float value = disp.normalize () / maxDisp;
                //std::cout << value << std::endl;

                const float hue = 180.0f * acos ( Vec3Df::dotProduct ( disp, redDir ) ) / M_PI; 
                if ( disp [1] < 0 ) {
                    c.FromHSV ( 360.0f-hue, 1.0f, value ); 
                } else {
                    c.FromHSV ( hue, 1.0f, value ); 
                }

                disparityMap.SetChannelValue ( y, x,   RED, 255.f * c.Red () );
                disparityMap.SetChannelValue ( y, x, GREEN, 255.f * c.Green () );
                disparityMap.SetChannelValue ( y, x,  BLUE, 255.f * c.Blue () );
            }
        }
        disparityMap.WriteToFile ( Config::OutputPath() + "Pyramids/01/disparity_l" + toString ( i ), PIXMAP | ASCII );
    }
    m_refImgPyr.Export ( Config::OutputPath() + "Pyramids/01/refImg" );
    m_refDepPyr.Export ( Config::OutputPath() + "Pyramids/01/refDep" );
    m_tarImgPyr.Export ( Config::OutputPath() + "Pyramids/01/tarImg" );
    m_tarDepPyr.Export ( Config::OutputPath() + "Pyramids/01/tarDep" );
    m_dispX.Export ( Config::OutputPath() + "Pyramids/01/dX" );
    m_dispY.Export ( Config::OutputPath() + "Pyramids/01/dY" );
}


/* Calculate  adjustment in the direction 
   of improved photometric-consistency */
float PixelTracker::computeDpX(
    float e_1, float e0, float e1, CartesianCoordinate p,CartesianCoordinate q
){
  if(e_1 < e0 && e_1 < e1)
    return (p.x - q.x - 0.5);
  if(e0 < e_1 && e0 < e1)
    return (p.x - q.x - 0.5 * ( (e_1 - e1) / (e_1 + e1 - 2*e0)));
  else
    return (p.x - q.x + 0.5);
}
float PixelTracker::computeDpY(
    float e_1, float e0, float e1, CartesianCoordinate p,CartesianCoordinate q
){
  if(e_1 < e0 && e_1 < e1)
    return (p.x - q.x - 0.5);
  if(e0 < e_1 && e0 < e1)
    return (p.x - q.x - 0.5 * ( (e_1 - e1) / (e_1 + e1 - 2*e0)));
  else
    return (p.x - q.x + 0.5);
}

float computeDp(
    float e_1, float e0, float e1, int p, int q
){
  if(e_1 < e0 && e_1 < e1)
    return (p - q - 0.5);
  if(e1 < e_1 && e1 < e0)
    return (p - q + 0.5);
  if(e0 < e_1 && e0 < e1)
    return (p - q + 0.5 * ( (e_1 - e1) / (e_1 + e1 - 2*e0)));
  return 0;
}

/* Calculate adjustment in the direction
   of improved surface-consistency*/
float PixelTracker::computeDsX(CartesianCoordinate p)
{

  float dx_1y = m_disparityMapX(p.y    , p.x - 1);
  float dx1y  = m_disparityMapX(p.y    , p.x + 1);
  float dxy_1 = m_disparityMapX(p.y - 1, p.x    );
  float dxy   = m_disparityMapX(p.y    , p.x    );
  float dxy1  = m_disparityMapX(p.y + 1, p.x    );

  float wx = exp( - pow((abs(dx_1y - dxy) - abs(dx1y-dxy)),2));

  float wy = exp( - pow((abs(dxy_1 - dxy) - abs(dxy1-dxy)),2));
    
  return (wx * ( dx_1y + dx1y ) + wy * ( dxy_1 + dxy))/ (2 * (wx + wy) );
}

float PixelTracker::computeDsY(CartesianCoordinate p)
{

  float dx_1y = m_disparityMapY(p.y    , p.x - 1);
  float dx1y  = m_disparityMapY(p.y    , p.x + 1);
  float dxy_1 = m_disparityMapY(p.y - 1, p.x    );
  float dxy   = m_disparityMapY(p.y    , p.x    );
  float dxy1  = m_disparityMapY(p.y + 1, p.x    );

  float wx = exp( - pow((abs(dx_1y - dxy) - abs(dx1y-dxy)),2));
  float wy = exp( - pow((abs(dxy_1 - dxy) - abs(dxy1-dxy)),2));

  return (wx * ( dx_1y + dx1y ) + wy * ( dxy_1 + dxy))/ (2 * (wx + wy) );
}

float PixelTracker::computeUs(
    CartesianCoordinate p, Image* iTarget, int iMode
){
    float Us = 0.0f;
    float sigma = 1.0f;/* 1 mm? */
    int width  = m_disparityMapX.cols(); 
    int height = m_disparityMapX.rows(); 

    for(int x = -1; x <= 1; x++){
        for(int y = -1; y <= 1; y++){

            /*Calculation made only with 8 neighboring pixels */
            if(x==0 && y == 0) continue;

            float delta = m_depthMap -> GetGreyLvl (p.y + y,p.x + x) 
                        - m_depthMap -> GetGreyLvl (p.y,p.x);

            float exponential = exp( -(delta*delta)/(sigma * sigma) ); 

            CartesianCoordinate q (p.x +x, p.y + y);
            float error = ((1 - m_refImage->PixelCorrelation (*iTarget ,p, q, 3, 3 ) ) / 2); 

            float disparity = 0.f;
            if (p.y + y >= 0 && p.x + x >=0 && 
                    p.y + y < height && p.x + x < width) {
                if (iMode == 1) {
                    disparity = m_disparityMapY(p.y + y, p.x + x);
                } else {
                    disparity = m_disparityMapX(p.y + y, p.x + x);
                }
            }
            disparity = isinf(disparity) ? 0 : disparity;

            Us += disparity * exponential * (1 - error); 
        }
    }
    return Us;
}
  
/* Calculate data-driven parameter wp*/
float PixelTracker::computeWp(float e_1, float e0, float e1)
{
  if(e_1 < e0 && e_1 <= e1)
    return (e0 - e_1);
  if(e1 <= e_1 && e1 < e0)
    return (e0 - e1);
  else
    return (0.5 * (e_1 + e1 - 2 * e0));
}

/*  user-specified smoothness parameter ws */ 
float PixelTracker::getWs()
{
  /*Value based on the paper*/
  return 0.005f;
}


/* Calculate refinament adjustment */ 
float PixelTracker::computeDprimeX(
   float e_1, float e0, float e1,CartesianCoordinate p,CartesianCoordinate q, Image * iTarget
){
  float dp = computeDp(e_1, e0, e1,p.x, q.x);
  //float ds = computeDsX(p);
  float us = computeUs(p, iTarget, 0);
  float wp = computeWp(e_1, e0, e1);
  float ws = getWs();

  /* return d'*/
  float dprime = (wp * dp + ws * us) / (wp + ws);

  if (my_isnan(dprime)) {
      scream();
  }
  return dprime;
}

/* Calculate refinament adjustment */ 
float PixelTracker::computeDprimeY(
    float e_1, float e0, float e1,CartesianCoordinate p, CartesianCoordinate q, Image* iTarget
){
  float dp = computeDp(e_1, e0 , e1,p.y,q.y);
  //float ds = computeDsY(p);
  float us = computeUs(p, iTarget, 1); // Mode = 1, use Y displacement Map
  float wp = computeWp(e_1, e0 , e1);
  float ws = getWs();

  /* return d'*/
  float dprime = (wp * dp + ws * us) / (wp + ws);

  if (my_isnan(dprime)) {
      scream();
  }
  return dprime;
}

/* Interative rafinement: 
   Stops when convergence is achieved
*/
void PixelTracker::disparityRefinement(Image * iTarget)
{
    int nbInteraction = 180; /* 180 for high resolution */
    static const float inf = std::numeric_limits<float>::infinity();
    int width   = m_disparityMapX.cols();
    int height  = m_disparityMapX.rows();

    Eigen::MatrixXf newDisparityMapX;
    Eigen::MatrixXf newDisparityMapY;

    newDisparityMapX.resize(height,width);
    newDisparityMapY.resize(height,width);

    for(int i = 0; i< nbInteraction; i++){
        /* for each pixel in disparity map calculated dprime pixel */
        for(int x = 0; x < m_disparityMapX.cols(); x++){
            for(int y = 0; y < m_disparityMapX.rows(); y++){

                float e_1 = 0.0f, e0 = 0.0f, e1 = 0.0f;

                float dispX = 0;
                float dispY = 0;
                if (
                        !isinf ( m_disparityMapX ( y, x ) ) 
                    &&  !isinf ( m_disparityMapY ( y, x ) )
                    //&&  m_disparityMapY ( y, x ) <  height 
                    //&&  m_disparityMapX ( y, x ) <  width
                    //&&  m_disparityMapY ( y, x ) > -height 
                    //&&  m_disparityMapX ( y, x ) > -width
                ) {
                    //std::cout << abs(m_disparityMapY ( y, x )) << std::endl;
                    //std::cout << abs(m_disparityMapX ( y, x )) << std::endl;
                    dispX = m_disparityMapX(y,x);
                    dispY = m_disparityMapY(y,x);
                } else {
                    newDisparityMapX(y,x) = inf;
                    newDisparityMapY(y,x) = inf;
                    continue;
                }

                /*Getting points to evaluate*/
                CartesianCoordinate p (x, y);

                int q_1x = (int)floor(x + dispX - 1);
                int q_1y = (int)floor(y + dispY);
                CartesianCoordinate q_1h (q_1x,q_1y);

                int qx = (int)floor(x + dispX);
                int qy = (int)floor(y + dispY);
                CartesianCoordinate q (qx,qy);

                int q1x = (int)floor(x + dispX + 1);
                int q1y = (int)floor(y + dispY);
                CartesianCoordinate q1h (q1x, q1y);

                /* Calculate mean [NCC] = (1 - NCC) / 2 */
                e_1 = (1 - m_refImage->PixelCorrelation (*iTarget ,p, q_1h ,3 ,3 ))/2;
                e0  = (1 - m_refImage->PixelCorrelation (*iTarget ,p, q   ,3 ,3 ))/2;
                e1  = (1 - m_refImage->PixelCorrelation (*iTarget ,p, q1h ,3 ,3 ))/2;

                /* Calculates new value for each disparity value*/
                newDisparityMapX(y,x) = computeDprimeX(e_1, e0, e1, p, q, iTarget);
                //if ( isnan (newDisparityMapX ( y, x )) ) {
                //    std::cout << "BUG X " << x << " " << y << " " << newDisparityMapX ( y, x ) << std::endl;
                //}

                q_1x = (int)floor(x + dispX);
                q_1y = (int)floor(y + dispY - 1 );
                CartesianCoordinate q_1v (q_1x,q_1y);

                q1x = (int)floor(x + dispX);
                q1y = (int)floor(y + dispY + 1);
                CartesianCoordinate q1v (q1x, q1y);

                /* Calculate mean [NCC] = (1 - NCC) / 2 */
                e_1 = ( 1 - m_refImage->PixelCorrelation (*iTarget ,p, q_1v,3 ,3 ))/2;
                e1  = ( 1 - m_refImage->PixelCorrelation (*iTarget ,p, q1v ,3 ,3 ))/2;

                static unsigned int nanCount = 0;
                newDisparityMapY(y,x) = computeDprimeY(e_1, e0, e1, p, q, iTarget);
                //if ( isnan (newDisparityMapY ( y, x )) ) {
                //    nanCount++;
                //    std::cout << "BUG Y " << x << " " << y << " " << newDisparityMapY ( y, x ) << " " << nanCount << std::endl;
                //}
            }
        }
        m_disparityMapX = newDisparityMapX;
        m_disparityMapY = newDisparityMapY;
    }
}

/* GDB function call displayDisparityMapX() */
void PixelTracker::displayDisparityMapX()
{
    int width = m_disparityMapX.cols();
    int height = m_disparityMapX.rows();
    
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            
            if (
                !isinf ( m_disparityMapX ( y, x ) ) 
                ) {
                if( m_disparityMapX ( y, x ) > height 
                    || m_disparityMapX ( y, x ) < -height 
                    ){
                    std::cout << "Strange value" << m_disparityMapY ( y, x )  << " ";
                }
            }
        }
//        std::cout << std::endl;
    }
}


/* GDB function */
void PixelTracker::displayDisparityMapY()
{
    int width = m_disparityMapY.cols();
    int height = m_disparityMapY.rows();
    
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){

            if (
                !isinf ( m_disparityMapY ( y, x ) ) 
                ) {
                if( m_disparityMapY ( y, x ) > height 
                    || m_disparityMapY ( y, x ) < -height 
                    ){

                std::cout << "Strange value" << m_disparityMapY ( y, x )  << " ";
                }
            }
        }
        //       std::cout << std::endl;
    }
}

