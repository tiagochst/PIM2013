#include <iostream>
#include <fstream>
#include "Image.h"
#include "SubImage.h"
#include "ImageBase.h"
#include "MathUtils.h"
#include "Rectangle.h"

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

float ImageBase::CalculateErrorScore (
    ImageBase&      iImageA,
    ImageBase&      iImageB
) {
    assert ( iImageA.GetWidth  () == iImageB.GetWidth  () );
    assert ( iImageA.GetHeight () == iImageB.GetHeight () );

    const unsigned int& SAMPLING_STEP   = 20;
    const int&          NH_SZ           = 9;

    float globalScore = 0.f;
    const unsigned int& height  = iImageA.GetHeight ();
    const unsigned int& width   = iImageA.GetWidth  ();
    for ( int y = NH_SZ / 2; y < height - NH_SZ / 2; y += SAMPLING_STEP ) {
        for ( int x = NH_SZ / 2; x < width - NH_SZ / 2; x += SAMPLING_STEP ) {
            float localScore    = 0.f;
            float denomA        = 0.f;
            float denomB        = 0.f;

            for ( int yy = -NH_SZ / 2; yy <= NH_SZ / 2; yy++ ) {
                for ( int xx = -NH_SZ / 2; xx <= NH_SZ / 2; xx++ ) {
                    const float valA = iImageA.GetGreyLvl ( y + yy, x + xx );
                    const float valB = iImageB.GetGreyLvl ( y + yy, x + xx );

                    denomA      += ( valA * valA );
                    denomB      += ( valB * valB );
                    localScore  += ( valA * valB );
                }
            }
            //std::cout << denomA << " " << denomB << std::endl;
            if ( denomA && denomB ) {
                localScore /= sqrt ( denomA * denomB );
            } else if ( !denomA && !denomB ) {
                localScore = 1.f; 
            } else {
                localScore = 0.f;
            }

            globalScore += ( 1.f - localScore );
        }
    }

    return ( globalScore );
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
    const Rectangle& sw( iSearchWindow );

    if ( oCorrelationMap != NULL ) {
        if ( oCorrelationMap->GetHeight() != sw.Height() ) {
            oCorrelationMap->SetHeight( sw.Height() );
        }
        if ( oCorrelationMap->GetWidth() != sw.Width() ) {
            oCorrelationMap->SetWidth( sw.Width() );
        }
        //if ( oCorrelationMap->GetMaxGreyLevel() != GetMaxGreyLevel() ) {
            oCorrelationMap->SetMaxGreyLevel( 255 );
        //}
    }
    oBestMatch.x = oBestMatch.y = -1;

    float maskDenom = 0;
    #pragma omp parallel for
    for ( int xx = 0; xx < iMask.GetWidth(); xx++ ) {
        #pragma omp parallel for
        for ( int yy = 0; yy < iMask.GetHeight(); yy++ ) {
            float maskVal = iMask.GetGreyLvl (
                yy,
                xx
            );
            
            #pragma omp critical
            maskDenom += ( maskVal * maskVal ); 
        }
    }
    
    float bestMatchVal = -1;
    CartesianCoordinate maskCenter = iMask.Center();
    #pragma omp parallel for
    for ( int x = sw.X(); x < sw.Right(); x++ ) {
        #pragma omp parallel for
        for ( int y = sw.Y(); y < sw.Bottom(); y++ ) {
            float val = 0;
            float myDenom = 0;
            for ( int xx = 0; xx < iMask.GetWidth () ; xx++ ) {
                for ( int yy = 0; yy < iMask.GetHeight (); yy++ ) {
                    float myVal   = GetGreyLvl (
                        y + yy - maskCenter.y,
                        x + xx - maskCenter.x
                    );
                    float maskVal = iMask.GetGreyLvl (
                        yy,
                        xx
                    );

                    myDenom += myVal * myVal; 
                    val     += myVal * maskVal;
                }
            }
            if ( maskDenom && myDenom ) {
                val /= sqrt ( maskDenom * myDenom );
            } else if ( !maskDenom && !myDenom ) {
                val = 1.f; 
            } else {
                val = 0.f;
            }
            
            if ( oCorrelationMap != NULL ) {
                oCorrelationMap->SetGreyLvl (
                    y - sw.Y(), 
                    x - sw.X(),
                    val * 255.f
                );
            }
            #pragma omp critical
            {
                if (
                    val >= bestMatchVal
                ) {
                    bestMatchVal = val;
                    oBestMatch.x = x;
                    oBestMatch.y = y;
                }
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

    float correlation   = 0.f;
    float myDenom       = 0.f;
    float otherDenom    = 0.f;
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

            const float myVal       = GetGreyLvl (
                y,
                x
            );
            const float otherVal    = iOther.GetGreyLvl (
                y,
                x
            );

            myDenom     += ( myVal * myVal );
            otherDenom  += ( otherVal * otherVal );
            correlation += ( otherVal * myVal );
        } 
    } 
    correlation /= sqrt ( myDenom * otherDenom );

    return ( correlation );
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
#ifdef IMG_EQ_SIMG
    oSubImage.SetMaxGreyLevel ( GetMaxGreyLevel () );

    for ( int i = iX; i < iX + iWidth; i++ ) {
        for ( int j = iY; j < iY + iHeight; j++ ) {
            int value = GetGreyLvl ( j, i );
            oSubImage.SetGreyLvl (
                j - iY,
                i - iX,
                value
            );        
        }
    }

#else
    oSubImage.SetOffset ( iX, iY );
    oSubImage.SetParent ( this );
#endif
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

    oDisplacementMapX.SetMaxGreyLevel ( iWindowWidth );
    oDisplacementMapY.SetMaxGreyLevel ( iWindowHeight );

    const int& maxAbsDispX = iWindowWidth / 2;
    const int& maxAbsDispY = iWindowHeight / 2; 
    CartesianCoordinate nCenter ( iNeighbourhoodWidth / 2, iNeighbourhoodHeight / 2 ); 
    CartesianCoordinate wCenter ( iWindowWidth / 2, iWindowHeight / 2 );

    SubImage neighbourhoodL;
    SubImage neighbourhoodR;

    #pragma omp parallel for
    for ( int x = 0; x < iRefImage.GetWidth(); x++ ) {
        #pragma omp parallel for private (neighbourhoodR, neighbourhoodL) 
        for ( int y = 0; y < iRefImage.GetHeight(); y++ ) {
            CartesianCoordinate bestMatchL;
            CartesianCoordinate bestMatchR;
            Image correlationMapL;
            Image correlationMapR; 

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

            Rectangle wRegionL( x - wCenter.x,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );
            Rectangle wRegionR( x - wCenter.x + 1,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );

            iTargetImage.TemplateMatch (
                neighbourhoodL,
                wRegionL,
                bestMatchL,
                &correlationMapL
            );
            iTargetImage.TemplateMatch (
                neighbourhoodR,
                wRegionR,
                bestMatchR,
                &correlationMapR
            );
            
            static const float inf = std::numeric_limits<float>::infinity();
            float bestR = -inf;
            float bestL = -inf;
            int valX = maxAbsDispX + 1;
            int valY = maxAbsDispY + 1;
            for ( int i = 0; i < correlationMapL.GetWidth (); i++ ) {
                for ( int j = 0; j < correlationMapL.GetHeight (); j++ ) {
                    float leftVal  = correlationMapL.GetGreyLvl ( j, i );
                    float rightVal = correlationMapR.GetGreyLvl ( j, i );
    
                    if (
                            leftVal >= 0.85f
                        &&  leftVal >= bestL
                        && rightVal >  bestR
                    ) {
                        valX = i + wRegionL.X() - x;
                        valY = j + wRegionL.Y() - y;

                        bestL = leftVal;
                        bestR = rightVal;
                    }
                }
            }
            
            valX = (valX >= 0) ? valX + 1 : valX;
            valY = (valY >= 0) ? valY + 1 : valY;
            oDisplacementMapX.SetGreyLvl( y, x, ( valX + maxAbsDispX ) );
            oDisplacementMapY.SetGreyLvl( y, x, ( valY + maxAbsDispY ) );
        }
    }
}

