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
#include "PointSet.h"

#include <csignal>
#include <cmath>
#include <stdexcept>

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
    Image*                  iRefDepth,
    Image*                  iTarDepth,
    const Eigen::Matrix3f&  iNbh,
    const Eigen::Matrix3f&  iErr
) {
    float Us = 0.0f;
    float sigma = 1000.0f;/* 10 cm? */
    float sumCoefs = 0.0f;

    unsigned int errone = 0;
    unsigned int expzero = 0;

    CartesianCoordinate p ( px, py );
    #pragma omp critical
    {
        for ( int x = -1; x <= 1; x++ ) {
            for ( int y = -1; y <= 1; y++ ) {
                CartesianCoordinate q ( px + x, py + y );

                /*Calculation made only with 8 neighboring pixels */
                //if ( !x && !y ) continue;

                float delta = ( iRefDepth->GetGreyLvl ( q ) )
                    - ( iRefDepth->GetGreyLvl ( p ) );

                float exponential = exp ( - ( delta * delta ) / ( sigma * sigma ) );
                float error = iErr ( y + 1, x + 1 );
                if ( error > 1.0 || error < 0.0f ) {
                    std::cout << "invalid error! " << error << std::endl; 
                }

                float disparity = iNbh ( y + 1, x + 1 );
                disparity = isinf ( disparity ) ? 0.0f : disparity;

                float coef = exponential * ( 1.0f - error );
                if ( exponential == 0.0f ) {
                    expzero++;
                    //#pragma omp critical 
                    //{
                    //    std::cout << "exp: " << exponential << " " << delta << std::endl;
                    //}
                }

                sumCoefs += coef;

                Us += disparity * coef;
            }
        }
        if (sumCoefs == 0) {
            std::cout << "error == 1 count: " << errone << std::endl;
            std::cout << "exp == 0 count: " << expzero << std::endl;
            Us = 0.0f;
        }
    }

    return Us / sumCoefs;
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
    const float&            px,
    const float&            py,
    const float&            qx,
    const float&            qy,
    Image*                  iRefDepth,
    Image*                  iTarDepth,
    const Eigen::Matrix3f&  iErr,
    const Eigen::Matrix3f&  iNbhX,
    const Eigen::Matrix3f&  iNbhY
) {
    float dpx = ComputeDp ( iErr ( 1, 0 ), iErr ( 1, 1 ) , iErr ( 1, 2 ), px, qx );
    float dpy = ComputeDp ( iErr ( 0, 1 ), iErr ( 1, 1 ) , iErr ( 2, 1 ), py, qy );

    float usx = ComputeUs ( px, py, iRefDepth, iTarDepth, iNbhX, iErr );
    float usy = ComputeUs ( px, py, iRefDepth, iTarDepth, iNbhY, iErr );

    float wpx = ComputeWp ( iErr ( 1, 0 ), iErr ( 1, 1 ), iErr ( 1, 2 ) );
    float wpy = ComputeWp ( iErr ( 0, 1 ), iErr ( 1, 1 ), iErr ( 2, 1 ) );

    //std::cout   <<  "   px:  " << std::setw (5)  << px
    //            <<  "   py:  " << std::setw (5)  << py
    //            <<  "   qx:  " << std::setw (5)  << qx
    //            <<  "   qy:  " << std::setw (5)  << qy
    //            <<  "   usx: " << std::setw (12)  << usx
    //            <<  "   usy: " << std::setw (12)  << usy
    //            <<  "   dpx: " << std::setw (12)  << dpx
    //            <<  "   dpy: " << std::setw (12)  << dpy
    //            <<  "   wpx: " << std::setw (12)  << wpx
    //            <<  "   wpy: " << std::setw (12)  << wpy
    //            <<  std::endl;

    const float ws = GetWs ();

    /* return d'*/
    Vec3Df dprime;
    dprime[0]  = ( wpx * dpx + ws * usx ) / ( wpx + ws );
    dprime[1]  = ( wpy * dpy + ws * usy ) / ( wpy + ws );

    return dprime;
}


PixelTracker::PixelTracker ()
    :   m_winHeight ( 1 ),
        m_winWidth ( 1 ),
        m_nbhHeight ( 9 ),
        m_nbhWidth ( 9 ),
        m_rejectionTreshold ( 0.f ),
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

    m_displacementX.resize (
        refImage->GetHeight (),
        refImage->GetWidth ()
    );
    m_displacementY.resize (
        refImage->GetHeight (),
        refImage->GetWidth ()
    );
    m_displacementZ.resize (
        refImage->GetHeight (),
        refImage->GetWidth ()
    );
    m_displacementX.fill(0);
    m_displacementY.fill(0);
    m_displacementZ.fill(0);


    // Processes pyramids from top to bottom. The coarsest level is processed 
    // once independently so we can estimate extremal motions on each direction.
    Image* ref  = m_refImgPyr.Top ();
    Image* tar  = m_tarImgPyr.Top ();
    Image* dX   = m_dispX.Top ();
    Image* dY   = m_dispY.Top ();

    int mx=INT_MIN, my=INT_MIN, mx_=INT_MAX, my_=INT_MAX;
    // Track a fixed set of features from ref to tar.
    //for ( int i = 3; i < ref->GetWidth () - 3; i += 15 ) {
    //    for ( int j = 3; j < ref->GetHeight () - 3; j += 15 ) {
    //        Image nbh;

    //        // Tracks a patch of size 3x3 on target image.
    //        ref->GetSubImage (
    //            i - 3, j - 3,
    //            7, 7,
    //            nbh
    //        );
    //        CartesianCoordinate best;
    //        tar->TemplateMatch ( nbh, best );

    //        std::cout << "(i,j) = " << i    << " " << j << std::endl;
    //        std::cout << "best = "  << best.x << " " << best.y << std::endl;
    //        std::cout << "delta = " << best.x - i << " " << best.y - j << std::endl;
 
    //        // Extremal movement estimation.
    //        mx  = max ( mx, best.x - i ); 
    //        my  = max ( my, best.y - j ); 
    //        mx_ = min ( mx_, best.x - i ); 
    //        my_ = min ( my_, best.y - j ); 
    //    }
    //}
    // Initial window size calculation.
    //m_winHeight = max ( my - my_, 3 );
    //m_winWidth  = max ( mx - mx_, 3 );
    m_rejectionTreshold = 0.65f;

    // Process the rest of the pyramid.
    for ( int i = m_refImgPyr.GetNumLevels () - 1; i >= 0; i-- ) {
        m_winWidth = m_winHeight = 3 + i;

        PyramidTrack ( i );

        std::cout << m_winWidth << " " << m_winHeight << std::endl;

        // Every level but the coarsest uses a 3x3 search window for pixel tracking.
        //if ( i == ( m_refImgPyr.GetNumLevels () - 1 ) ) {
        //}
    }
}

static unsigned int UNMATCHED = 0;
void PixelTracker::PyramidTrack (
    const unsigned int&     iLevel
) {
    UNMATCHED = 0;

    PyramidMatch        ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels." << std::endl;
    PyramidSmooth       ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels." << std::endl;
    PyramidOrdering     ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels." << std::endl;
    PyramidUniqueness   ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels." << std::endl;
    PyramidRematch      ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels." << std::endl;
    PyramidUniqueness   ( iLevel );
    std::cout << UNMATCHED << " unmatched pixels. Rematching..." << std::endl;
    const unsigned int maxIter = 80u;
    const unsigned int minIter = 20u;
    const unsigned int nIter = maxIter - ( (maxIter-minIter) * (iLevel+1) ) / m_refImgPyr.GetNumLevels ();
    //const unsigned int nIter = 10;
    //PyramidRefineStep0  ( iLevel );
    for ( unsigned int i = 0; i < nIter; i++ ) {
        PyramidRefine       ( iLevel );
    }
    PyramidUniqueness   ( iLevel );
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
    
    static const float inf = std::numeric_limits<float>::infinity();
    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            int deltaX = 0;
            int deltaY = 0;
            if ( iLevel < m_refImgPyr.GetNumLevels () - 1 ) {
                float fdX = m_dispX[iLevel + 1]->GetNormed ( j/2, i/2 ) * 2.0f;
                float fdY = m_dispY[iLevel + 1]->GetNormed ( j/2, i/2 ) * 2.0f;
                if ( isinf(fdX) && isinf(fdY) ) {
                    if ( IsOdd ( j ) ) {
                        fdX = m_dispX[iLevel + 1]->GetNormed ( (j+1)/2, i/2) * 2.0f; 
                        fdY = m_dispY[iLevel + 1]->GetNormed ( (j+1)/2, i/2) * 2.0f; 
                    } else {
                        fdX = m_dispX[iLevel + 1]->GetNormed ( (j-1)/2, i/2) * 2.0f; 
                        fdY = m_dispY[iLevel + 1]->GetNormed ( (j-1)/2, i/2) * 2.0f; 
                    }
                    if ( IsOdd ( i ) ) {
                        fdX = m_dispX[iLevel + 1]->GetNormed ( j/2, (i+1)/2) * 2.0f; 
                        fdY = m_dispY[iLevel + 1]->GetNormed ( j/2, (i+1)/2) * 2.0f; 
                    } else {
                        fdX = m_dispX[iLevel + 1]->GetNormed ( j/2, (i-1)/2) * 2.0f; 
                        fdY = m_dispY[iLevel + 1]->GetNormed ( j/2, (i-1)/2) * 2.0f; 
                    }
                    if ( !isinf(fdX) || !isinf(fdY) ) {
                        fdX = fdY = 0.0f;
                    }
                    deltaX = nearbyint(fdX); 
                    deltaY = nearbyint(fdY); 
                } else {
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
            float score = tar->TemplateMatch (
                nbh,
                i - m_winWidth / 2  + deltaX,
                j - m_winHeight / 2 + deltaY,
                m_winWidth,
                m_winHeight,
                best
            );

            if ( score < m_rejectionTreshold ) {
                #pragma omp atomic
                UNMATCHED++;

                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
            } else {
                newDX->SetNormed ( j, i, best.x - i ) ;
                newDY->SetNormed ( j, i, best.y - j ) ;
            }
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidRematch (
    const unsigned int&     iLevel
) {
    static const float inf = std::numeric_limits<float>::infinity();

    Image* ref      = m_refImgPyr[iLevel]; 
    Image* tar      = m_tarImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );
    
    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            Image nbh;

            const float& dispX = dX->GetNormed ( j, i );
            const float& dispY = dY->GetNormed ( j, i );
            if ( !isinf(dispX) || !isinf(dispY) ) {
                newDX->SetNormed ( j, i, dispX ) ;
                newDY->SetNormed ( j, i, dispY ) ;
                continue;
            }

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
            float score = tar->TemplateMatch (
                nbh,
                i - m_winWidth / 2  + deltaX,
                j - m_winHeight / 2 + deltaY,
                m_winWidth,
                m_winHeight,
                best
            ); 

            if ( score < m_rejectionTreshold ) {
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
            } else {
                #pragma omp atomic
                UNMATCHED--;

                newDX->SetNormed ( j, i, best.x - i ) ;
                newDY->SetNormed ( j, i, best.y - j ) ;
            }
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

    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < width; i++ ) {
        for ( int j = 0; j < height; j++ ) {
            unsigned int count = 0;
            unsigned int total = 0;

            const float& dispX = dX->GetNormed ( j, i );
            const float& dispY = dY->GetNormed ( j, i );
            for ( int ii = i - 1; ii <= i + 1; ii++ ) {
                for ( int jj = j - 1; jj <= j + 1; jj++ ) {
                    if (ii != i && jj != j) continue;
                    unsigned int idX = ii - i + 1;
                    unsigned int idY = jj - j + 1;
                    if (
                            ( jj >= 0 && ii >= 0 )
                        &&  ( jj < height && ii < width )
                    ) {
                        float a = dX->GetNormed ( jj, ii ) - dispX;
                        float b = dY->GetNormed ( jj, ii ) - dispY;

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
                &&  ( (float)count / (float)total ) < 0.5f 
            ) {
                #pragma omp atomic
                UNMATCHED++;
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

    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < width; i++ ) {
        for ( int j = 0; j < height; j++ ) {
            const float& dispX  = dX->GetNormed (     j, i     );
            const float& dispY  = dY->GetNormed (     j, i     );
            const float& dispXX = dX->GetNormed (     j, i + 1 ) - dispX;
            const float& dispYX = dY->GetNormed (     j, i + 1 ) - dispY;
            const float& dispXY = dX->GetNormed ( j + 1, i     ) - dispX;
            const float& dispYY = dY->GetNormed ( j + 1, i     ) - dispY;

            if (
                     isinf ( dispX  ) ||  isinf ( dispY  )
                ||   isinf ( dispXX ) ||  isinf ( dispXY )
                ||   isinf ( dispYX ) ||  isinf ( dispYY )
                ||  ( dispXX > 1.0f ) || ( dispXY > 1.0f )
                ||  ( dispYX > 1.0f ) || ( dispYY > 1.0f )
            ) {
                if (
                        !isinf ( dispX ) 
                    &&  !isinf ( dispY ) 
                ) {
                    #pragma omp atomic
                    UNMATCHED++;
                }
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

    #pragma omp parallel for shared (newDX, newDY)
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
                    abs ( best.x - i ) <= 1.0f
                &&  abs ( best.y - j ) <= 1.0f
            ) {
                newDX->SetNormed ( j, i, dispX ) ;
                newDY->SetNormed ( j, i, dispY ) ;
            } else {
                UNMATCHED++;
                newDX->SetNormed ( j, i, inf ) ;
                newDY->SetNormed ( j, i, inf ) ;
            }
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::PyramidRefineStep0 (
    const unsigned int&     iLevel
) {
    Image* ref      = m_refImgPyr[iLevel]; 
    Image* dX       = m_dispX[iLevel]; 
    Image* dY       = m_dispY[iLevel]; 
    Image* newDX    = new Image ( dX->GetWidth (), dX->GetHeight (), dX->GetMaxGreyLevel () );
    Image* newDY    = new Image ( dY->GetWidth (), dY->GetHeight (), dY->GetMaxGreyLevel () );

    static const float inf = std::numeric_limits<float>::infinity();
    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            float dispX = dX->GetNormed ( j, i );
            float dispY = dY->GetNormed ( j, i );
            if ( my_isnan ( dispX ) || my_isnan ( dispX ) ) {
                std::cout   << "RS0- dX: " << std::setw (12) << dispX
                            << "RS0- dY: " << std::setw (12) << dispY;
                raise(SIGABRT);
            }
            if ( isinf ( dispX ) || isinf ( dispY ) ) {
                dispX = dispY = 0;
                for ( int ii = -1; ii <= 1; ii++ ) {
                    for ( int jj = -1; jj <= 1; jj++ ) {
                        if ( !ii && !jj ) continue;

                        float valX = dX->GetNormed ( jj + j, ii + i );
                        float valY = dY->GetNormed ( jj + j, ii + i );

                        if ( !isinf ( valX ) ) dispX += valX;
                        if ( !isinf ( valY ) ) dispY += valY;
                    }
                }
                dispX /= 8.0f;
                dispY /= 8.0f;
            }
            if ( my_isnan ( dispX ) || my_isnan ( dispX ) ) {
                std::cout   << "RS0+ dX: " << std::setw (12) << dispX
                            << "RS0+ dY: " << std::setw (12) << dispY;
                raise(SIGABRT);
            }
            newDX->SetNormed ( j, i, dispX );
            newDY->SetNormed ( j, i, dispY );
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
    #pragma omp parallel for shared (newDX, newDY)
    for ( int i = 0; i < ref->GetWidth (); i++ ) {
        for ( int j = 0; j < ref->GetHeight (); j++ ) {
            float dispX = dX->GetNormed ( j, i );
            float dispY = dY->GetNormed ( j, i );
            if (isinf(dispX) || isinf(dispY)) {
                dispX = 0;
                dispY = 0;
            }

            Eigen::Matrix3f nbhX; // 3x3 neighbourhood of X movement
            Eigen::Matrix3f nbhY; // 3x3 neighbourhood of Y movement
            Eigen::Matrix3f err;  // /NCC function
            nbhX.fill ( 0 );
            nbhY.fill ( 0 );
            err.fill ( 0 );

            CartesianCoordinate p ( i, j );
            for ( int ii = -1; ii <= 1; ii++ ) {
                for ( int jj = -1; jj <= 1; jj++ ) {
                    CartesianCoordinate q (
                        nearbyint ( i + dispX ) + ii,
                        nearbyint ( j + dispY ) + jj
                    );
                    float ncc = ref->PixelCorrelation ( *tar, p , q, 3, 3 );
                    if ( my_isnan(ncc) || ncc > 1.0f ) {
                        std::cout << "INVALID NCC " << ncc << std::endl;
                        raise(SIGABRT);
                    }
                    float error = ( 1.0f - ncc ) / 2.0f;

                    float localDx = dX->GetNormed ( j + jj, i + ii );
                    float localDy = dY->GetNormed ( j + jj, i + ii );
                    nbhX ( jj + 1, ii + 1 ) = ( !isinf ( localDx ) ) ? localDx : 0.0f;
                    nbhY ( jj + 1, ii + 1 ) = ( !isinf ( localDy ) ) ? localDy : 0.0f;
                    err  ( jj + 1, ii + 1 ) = error;
                    if (
                            my_isnan ( nbhX ( jj + 1, ii + 1 ) )
                        ||  my_isnan ( nbhY ( jj + 1, ii + 1 ) )
                        ||  my_isnan ( err  ( jj + 1, ii + 1 ) )
                    ) {
                        std::cout
                        <<"nbhX ( "<<jj+1<<", "<<ii+1<<" )="<<nbhX ( jj + 1, ii + 1 )<< std::endl
                        <<"nbhY ( "<<jj+1<<", "<<ii+1<<" )="<<nbhY ( jj + 1, ii + 1 )<< std::endl
                        <<"err  ( "<<jj+1<<", "<<ii+1<<" )="<<err  ( jj + 1, ii + 1 )<< std::endl
                        << std::endl;
                        std::cout << i << " " << j << std::endl;
                        raise(SIGABRT);
                    }
                }
            }

            Vec3Df newDisp = ComputeDprime (
                i, j,
                nearbyint ( i + dispX ), nearbyint ( j + dispY ),
                m_refDepPyr[iLevel], m_tarDepPyr[iLevel], 
                err,
                nbhX,
                nbhY
            );

            //if ( fabs(newDisp[0] - dispX) >= 5.f ) {
            //    std::cout   << "nDX: " << std::setw(12) << newDisp[0]
            //                << std::endl
            //                << "oDX: " << std::setw(12) <<      dispX
            //                << std::endl;
            //}
            //if ( fabs(newDisp[1] - dispX) >= 5.f ) {
            //    std::cout   << "nDY: " << std::setw(12) << newDisp[0]
            //                << std::endl
            //                << "oDY: " << std::setw(12) <<      dispY
            //                << std::endl;
            //}
            
            newDX->SetNormed ( j, i, newDisp[0] ) ;
            newDY->SetNormed ( j, i, newDisp[1] ) ;
        }
    }
    m_dispX.SetLevel ( iLevel, newDX );
    m_dispY.SetLevel ( iLevel, newDY );
}

void PixelTracker::SetRejectionTreshold (
    const float&            iRejectionTreshold
) {
    m_rejectionTreshold = iRejectionTreshold;
}

void PixelTracker::Export (
    const std::string&      iFilename
) const {
    ExportPyramidLevel ( 0, iFilename );
}

void PixelTracker::CalculateMotionField (
    PointSet*     iRefMesh,
    PointSet*     iTarMesh
) {
    const Image* refDep = m_refDepPyr[0];
    const Image* tarDep = m_tarDepPyr[0];
    const Image* dispX  = m_dispX[0];
    const Image* dispY  = m_dispY[0];

    const unsigned int& width   = dispX->GetWidth ();
    const unsigned int& height  = dispX->GetHeight ();

    for ( unsigned int x = 0; x < width; x++ ) {
        for ( unsigned int y = 0; y < height; y++ ) {
            unsigned int refU = x;
            unsigned int refV = y;

            float dX = dispX->GetNormed ( y, x );
            float dY = dispY->GetNormed ( y, x );


            float minTarU = fmax ( 0, fmin ( width-1, floor((float)x + dX) ) );
            float maxTarU = fmax ( 0, fmin ( width-1,  ceil((float)x + dX) ) );

            float minTarV = fmax ( 0, fmin ( height-1, floor((float)y + dY) ) );
            float maxTarV = fmax ( 0, fmin ( height-1,  ceil((float)y + dY) ) );

            try {
                const Vec3Df& refPos = iRefMesh->GetVertex ( refU, refV )->GetPosition ();
                const Vec3Df& q0 = iTarMesh->GetVertex ( minTarU, minTarV )->GetPosition ();
                const Vec3Df& q1 = iTarMesh->GetVertex ( minTarU, maxTarV )->GetPosition ();
                const Vec3Df& q2 = iTarMesh->GetVertex ( maxTarU, minTarV )->GetPosition ();
                const Vec3Df& q3 = iTarMesh->GetVertex ( maxTarU, maxTarV )->GetPosition ();

                Vec3Df tarPos;
                Vec3Df displacement; 
                tarPos = BilinearInterpolation(q0,q1,q2,q3,minTarU,minTarV,maxTarU,maxTarV,x+dX,y+dY); 
                displacement = tarPos - refPos;

                m_displacementX ( y, x ) = displacement[0];
                m_displacementY ( y, x ) = displacement[1];
                m_displacementZ ( y, x ) = displacement[2];
            } catch ( std::out_of_range ex ) {
                std::cerr   << ex.what () << std::endl;
                std::cerr   << "Pixel info: " << std::endl;
                std::cerr   << "  - (  x,  y ) = " << x << " " << y << std::endl;
                std::cerr   << "  - ( dx, dy ) = " << dX << " " << dY << std::endl;
                std::cerr   << "  - region     = " << minTarU << " " << minTarV << " " << maxTarU << " " << maxTarV << std::endl;
            }
        }
    }
}

void PixelTracker::ExportPyramidLevel (
    const unsigned int&     iLevel,
    const std::string&      iPath
) const {
    system ( ( "mkdir -p " + iPath ).c_str () );

    static const float inf = std::numeric_limits<float>::infinity();

    const Image* dispX = m_dispX[iLevel];
    const Image* dispY = m_dispY[iLevel];
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

    PPMImage disparityMap;
    disparityMap.SetMaxValue ( 255u );
    disparityMap.ResetDimensions (
        dispX->GetWidth (),
        dispX->GetHeight ()        
    );

    std::ofstream motionField ( (iPath + "motionField.dat").c_str(), std::ofstream::binary );

    Color c;
    Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    for ( unsigned int x = 0; x < dispX->GetWidth (); x++ ) {
        for ( unsigned int y = 0; y < dispX->GetHeight (); y++ ) {
            const float& dX = dispX->GetNormed ( y, x );
            const float& dY = dispY->GetNormed ( y, x );
            Vec3Df disp ( dX, dY, 0.0f );

            if ( isinf(dX) || isinf(dY) ) {
                disparityMap.SetChannelValue ( y, x, GREY, 255u );
                continue;
            }

            const float value = disp.normalize () / maxDisp;

            const float hue = 180.0f * acos ( Vec3Df::dotProduct ( disp, redDir ) ) / M_PI; 
            if ( disp [1] < 0 ) {
                c.FromHSV ( 360.0f-hue, 1.0f, value ); 
            } else {
                c.FromHSV ( hue, 1.0f, value ); 
            }

            const float& rDX = m_displacementX ( y, x );
            const float& rDY = m_displacementY ( y, x );
            const float& rDZ = m_displacementZ ( y, x );
            motionField.write ( (const char*)&dX, sizeof ( float ) );
            motionField.write ( (const char*)&dY, sizeof ( float ) );
            motionField.write ( (const char*)&rDX, sizeof ( float ) );
            motionField.write ( (const char*)&rDY, sizeof ( float ) );
            motionField.write ( (const char*)&rDZ, sizeof ( float ) );

            disparityMap.SetChannelValue ( y, x,   RED, 255.f * c.Red () );
            disparityMap.SetChannelValue ( y, x, GREEN, 255.f * c.Green () );
            disparityMap.SetChannelValue ( y, x,  BLUE, 255.f * c.Blue () );
        }
    }
    motionField.close ();
    disparityMap.WriteToFile ( iPath + "disparityMap.ppm", PIXMAP | BINARY );
}
