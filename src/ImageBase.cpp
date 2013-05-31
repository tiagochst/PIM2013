#include <fstream>
#include "Image.h"
#include "SubImage.h"
#include "ImageBase.h"
#include "MathUtils.h"

void ImageBase::CreateAsciiPgm (
    const std::string&  iFilename
) {
    int i = 0, j = 0;
    std::ofstream ostr (
        iFilename.c_str()
    );

    /* writing header */
    ostr << "P2" << std::endl;
    ostr << "# CREATOR: PIM380 PROJ" << std::endl;;
    ostr << GetWidth () << " " << GetHeight () << std::endl;
    ostr << GetMaxGreyLevel () << std::endl;

    for ( i = 0; i < GetHeight (); i++ ) {
        for ( j = 0; j < GetWidth (); j++ ) {
            ostr << GetGreyLvl ( i, j ) << std::endl;
        }
    }

    ostr.close();
}

inline CartesianCoordinate ImageBase::Center () const 
{
    return CartesianCoordinate (
        GetWidth ()  / 2,
        GetHeight () / 2
    );
}

float ImageBase::TemplateMatch (
    const ImageBase&        iMask,
    CartesianCoordinate&    oBestMatch,
    ImageBase*              oCorrelationMap
) const {
    Rectangle window ( 0, 0, GetWidth (), GetHeight () );
    TemplateMatch (
        iMask,
        window,
        oBestMatch,
        oCorrelationMap
    );
}

float ImageBase::TemplateMatch(
    const ImageBase&        iMask,
    const Rectangle&        iSearchWindow,
    CartesianCoordinate&    oBestMatch,
    ImageBase*              oCorrelationMap
) const {
    const ImageBase& me = (*this);
    const Rectangle& sw( iSearchWindow );

    if ( oCorrelationMap != NULL ) {
        if ( oCorrelationMap->GetHeight() != sw.Height() ) {
            oCorrelationMap->SetHeight( sw.Height() );
        }
        if ( oCorrelationMap->GetWidth() != sw.Width() ) {
            oCorrelationMap->SetWidth( sw.Width() );
        }
        if ( oCorrelationMap->GetMaxGreyLevel() != GetMaxGreyLevel() ) {
            oCorrelationMap->SetMaxGreyLevel( GetMaxGreyLevel() );
        }
    }

    int nMaskElems = iMask.GetWidth () * iMask.GetWidth ();
    
    float maskDenom = 0;
    #pragma omp parallel for
    for ( int xx = 0; xx < iMask.GetWidth(); xx++ ) {
        #pragma omp parallel for
        for ( int yy = 0; yy < iMask.GetHeight(); yy++ ) {
            CartesianCoordinate maskCoords( xx, yy );
            
            float maskVal = iMask.GetNormed( maskCoords );
            
            #pragma omp critical
            maskDenom += maskVal * maskVal; 
        }
    }
    
    float bestMatchVal = 0;
    CartesianCoordinate maskCenter = iMask.Center();
    #pragma omp parallel for
    for ( int x = sw.X(); x < sw.Right(); x++ ) {
        #pragma omp parallel for
        for ( int y = sw.Y(); y < sw.Bottom(); y++ ) {
            float val = 0;
            float myDenom = 0;
            for ( int xx = -maskCenter.x; xx <= maskCenter.x ; xx++ ) {
                for ( int yy = -maskCenter.y; yy <= maskCenter.y; yy++ ) {
                    CartesianCoordinate myCoords( x + xx, y + yy );
                    CartesianCoordinate maskCoords( xx + maskCenter.x,
                                                    yy + maskCenter.y );

                    float myVal   = me.GetNormed( myCoords );
                    float maskVal = iMask.GetNormed( maskCoords );

                    myDenom += myVal * myVal; 
                    val     += myVal * maskVal;
                }
            }
            val /= sqrt ( maskDenom * myDenom );
            
            if ( oCorrelationMap != NULL ) {
                CartesianCoordinate corrCoords( x - sw.X(), y - sw.Y() );

                oCorrelationMap->SetNormed( corrCoords, val);
            }
            #pragma omp critical
            if ( val >= bestMatchVal ) {
                bestMatchVal = val;
                oBestMatch.x = x;
                oBestMatch.y = y;
            }
        }
    }

    return bestMatchVal;
}

float ImageBase::Correlation (
    const ImageBase& iOther
) const {
    if ( 
            iOther.GetWidth() != GetWidth ()
        ||  iOther.GetHeight() != GetHeight ()
    ) {
        throw IncompatibleImages();
    }

    float correlation = 0;
    for (
        int x = 0;
        x < GetWidth ();
        x++
    ) {
        for (
            int y = 0;
            y < GetHeight ();
            y++
        ) {
            CartesianCoordinate c ( x, y );

            correlation += GetNormed ( c ) 
                         * iOther.GetNormed ( c );
        } 
    } 
    
    return correlation;
}

void ImageBase::GetSubImage (
    const Rectangle&    iRegion,
    SubImage&           oSubImage
) {
    GetSubImage (
        iRegion.X (),
        iRegion.Y (),
        iRegion.Width (),
        iRegion.Height (),
        oSubImage
    );
}

void ImageBase::GetSubImage (
    const int&  iX,
    const int&  iY,
    const int&  iWidth,
    const int&  iHeight,
    SubImage&   oSubImage
) {
    oSubImage.SetDimensions ( iWidth, iHeight );
    oSubImage.SetOffset ( iX, iY );
    oSubImage.SetParent ( this );
}

void ImageBase::TrackPixels(
    ImageBase&          iRefImage,
    ImageBase&          iTargetImage,
    const int&          iWindowWidth,
    const int&          iWindowHeight,
    const int&          iNeighbourhoodWidth,
    const int&          iNeighbourhoodHeight,
    ImageBase&          oDisplacementMapX,
    ImageBase&          oDisplacementMapY
) {
    if (
            iRefImage.GetHeight() != iTargetImage.GetHeight()
        ||  iRefImage.GetWidth()  != iTargetImage.GetWidth()
    ) {
        throw IncompatibleImages();
    }

    CartesianCoordinate nCenter ( iNeighbourhoodWidth / 2, iNeighbourhoodHeight / 2 ); 

    SubImage neighbourhoodL;
    SubImage neighbourhoodR;

    #pragma omp parallel for
    for ( int x = 0; x < iRefImage.GetWidth(); x++ ) {
        #pragma omp parallel for private (neighbourhoodR, neighbourhoodL) 
        for ( int y = 0; y < iRefImage.GetHeight(); y++ ) {
            iRefImage.GetSubImage (
                x - nCenter.x,
                y - nCenter.y,
                iNeighbourhoodWidth,
                iNeighbourhoodHeight,
                neighbourhoodL
            );
            iRefImage.GetSubImage (
                x - nCenter.x + 1,
                y - nCenter.y,
                iNeighbourhoodWidth,
                iNeighbourhoodHeight,
                neighbourhoodR
            );

            CartesianCoordinate bestMatchL;
            Image correlationMapL ( iWindowWidth, iWindowHeight, 65535 );

            CartesianCoordinate bestMatchR;
            Image correlationMapR ( iWindowWidth, iWindowHeight, 65535 );

            CartesianCoordinate wCenter ( iWindowWidth / 2, iWindowHeight / 2 );

            Rectangle wRegionL( x - wCenter.x,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );
            Rectangle wRegionR( x - wCenter.x + 1,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );

            iTargetImage.TemplateMatch( neighbourhoodL, wRegionL, bestMatchL, &correlationMapL );
            iTargetImage.TemplateMatch( neighbourhoodR, wRegionR, bestMatchR, &correlationMapR );
            
            CartesianCoordinate bestMatch;

            static const float inf = std::numeric_limits<float>::infinity();
            float bestR = -inf;
            float bestL = -inf;
            for ( int i = 0; i < correlationMapL.GetWidth (); i++ ) {
                for ( int j = 0; j < correlationMapL.GetHeight (); j++ ) {
                    float leftVal  = correlationMapL.GetNormed ( j, i );
                    float rightVal = correlationMapR.GetNormed ( j, i );
    
                    if (
                            leftVal > bestL
                        && rightVal > bestR
                    ) {
                        bestMatch.x = i + wRegionL.X();
                        bestMatch.y = j + wRegionL.Y();                        

                        bestL = leftVal;
                        bestR = rightVal;
                    }
                }
            }

            int valX = (bestMatch.x - x);
            int valY = (bestMatch.y - y);
            
            oDisplacementMapX.SetGreyLvl( y, x, valX );
            oDisplacementMapY.SetGreyLvl( y, x, valY );
        }
    }

    const int& maxAbsDispX = oDisplacementMapX.GetMaxGreyLevel () / 2;
    const int& maxAbsDispY = oDisplacementMapY.GetMaxGreyLevel () / 2;
    #pragma omp parallel for
    for ( int x = 0; x < iRefImage.GetWidth(); x++ ) {
        #pragma omp parallel for
        for ( int y = 0; y < iRefImage.GetHeight(); y++ ) {

            int valX = oDisplacementMapX.GetGreyLvl( y, x );
            int valY = oDisplacementMapY.GetGreyLvl( y, x );

            oDisplacementMapX.SetGreyLvl( y, x, ( valX + maxAbsDispX ) );
            oDisplacementMapY.SetGreyLvl( y, x, ( valY + maxAbsDispY ) );
        }
    }
}
