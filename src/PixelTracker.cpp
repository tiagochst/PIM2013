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

#include <cmath>

PixelTracker::PixelTracker (
    const unsigned int&         iReferenceId
)   :   m_referenceId ( iReferenceId ),
        m_refImage ( (Image*)0x0 ),
        m_winHeight ( 1 ),
        m_winWidth ( 1 ),
        m_nbhHeight ( 1 ),
        m_nbhWidth ( 1 ),
        m_rejectionTreshold ( 0.f ),
        m_forwardTrack ( (TrackInfo*)0x0 ),
        m_backwardsTrack ( (TrackInfo*)0x0 ),
        m_disparityMapX (),
        m_disparityMapY ()
{}

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
                        unsigned int idX = xx - x + 1;
                        unsigned int idY = yy - y + 1;
                        if (
                                ( yy >= 0 && xx >= 0 )
                            &&  ( yy < height && xx < width )
                        ) {
                            float& a = ndX ( idY, idX ) = m_disparityMapX ( yy, xx );
                            float& b = ndY ( idY, idX ) = m_disparityMapY ( yy, xx );

                            minDispX = min ( a, minDispX );
                            minDispY = min ( b, minDispY );
                            maxDispX = max ( a, maxDispX );
                            maxDispY = max ( b, maxDispY );
                        }
                    }
                }
            }
            minDispX = ( isinf ( minDispX ) ) ? 0 : minDispX;
            minDispY = ( isinf ( minDispY ) ) ? 0 : minDispY;
            maxDispX = ( isinf ( maxDispX ) ) ? 0 : maxDispX;
            maxDispY = ( isinf ( maxDispY ) ) ? 0 : maxDispY;

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
                m_disparityMapX ( y, x ) = fmd.m_xCoord - x;
                m_disparityMapY ( y, x ) = fmd.m_yCoord - y;

                //if ( abs(fmd.m_xCoord >= m_winWidth / 2 ) )
                //    std::cerr << "Error DispX " << fmd.m_xCoord << " " << x << " " << y << std::endl;
                //if ( abs(fmd.m_yCoord >= m_winHeight / 2 ) )
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
                for ( int xx = x - 1; xx < x + 1; xx++ ) {
                    for ( int yy = y - 1; yy < y + 1; yy++ ) {
                        unsigned int idX = xx - x + 1;
                        unsigned int idY = yy - y + 1;
                        if (
                                ( yy >= 0 && xx >= 0 )
                            &&  ( yy < height && xx < width )
                        ) {
                            float& a = ndX ( idY, idX ) = m_disparityMapX ( yy, xx );
                            float& b = ndY ( idY, idX ) = m_disparityMapY ( yy, xx );

                            if ( sqrt ( a*a + b*b ) <= 1.0f ) {
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
            match &= ( ( fme + bme ).getLength() <= 1.0f );

            #pragma omp critical
            {
                if ( match ) {
                    unmatchedPixels--;
                    fmd.m_matched = true;
                    bmd.m_matched = true;
                } else {
                    m_disparityMapX ( y, x ) = inf;
                    m_disparityMapY ( y, x ) = inf;
                }
            }
        }
    }

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
            std::cout << c.Red () << " " << c.Green () << " " << c.Blue () << std::endl;

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

    PPMImage disparityMap;
    disparityMap.SetMaxValue ( 255u );
    disparityMap.ResetDimensions (
        m_refImage->GetWidth (),
        m_refImage->GetHeight ()        
    );
    //PPMImage colorChart;
    //printColorChart ( 30, 30, colorChart);

    const float maxDispX = m_winWidth  * 0.5f;   
    const float maxDispY = m_winHeight * 0.5f;
    const float maxDisp = sqrt ( maxDispX * maxDispX + maxDispY * maxDispY );

    Color c;
    Vec3Df redDir ( 1.0f, 0.0f, 0.0f );
    for ( unsigned int x = 0; x < m_refImage->GetWidth (); x++ ) {
        for ( unsigned int y = 0; y < m_refImage->GetHeight (); y++ ) {
            const float dX = m_disparityMapX ( y, x );
            const float dY = m_disparityMapY ( y, x );

            if ( dX > maxDispX || dY > maxDispY ) {
                disparityMap.SetChannelValue ( y, x, GREY, 255u );
                continue;
            }

            Vec3Df disp ( dX, dY, 0.0f );
            const float value = disp.normalize () / maxDisp;

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
    disparityMap.WriteToFile ( iFilename, PIXMAP | BINARY );
}
