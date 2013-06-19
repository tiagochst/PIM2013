#include <iostream>
#include <fstream>
#include "Image.h"
#include "SubImage.h"
#include "ImageBase.h"
#include "MathUtils.h"
#include "Rectangle.h"

#include <csignal>

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

    const unsigned int& SAMPLING_STEP   = 9;
    const int&          NH_SZ           = 9;

    float globalScore = 0.f;
    const int height  = iImageA.GetHeight ();
    const int width   = iImageA.GetWidth  ();
    unsigned int featureCount = 0;
    unsigned int zeroCount = 0;
    unsigned int oneCount = 0;
    for ( int y = NH_SZ / 2; y < height - NH_SZ / 2; y += SAMPLING_STEP ) {
        for ( int x = NH_SZ / 2; x < width - NH_SZ / 2; x += SAMPLING_STEP ) {
            float varA          = 0.0f;
            float varB          = 0.0f;
            float meanA         = 0.0f;
            float meanB         = 0.0f;
            unsigned int nPixels = NH_SZ * NH_SZ;

            for ( int yy = -NH_SZ / 2; yy <= NH_SZ / 2; yy++ ) {
                for ( int xx = -NH_SZ / 2; xx <= NH_SZ / 2; xx++ ) {
                    const float valA = iImageA.GetGreyLvl ( y + yy, x + xx );
                    const float valB = iImageB.GetGreyLvl ( y + yy, x + xx );

                    meanA       += valA;
                    meanB       += valB;
                }
            }
            meanA   /= (float)nPixels;
            meanB   /= (float)nPixels;

            float localScore    = 0.0f;
            for ( int yy = -NH_SZ / 2; yy <= NH_SZ / 2; yy++ ) {
                for ( int xx = -NH_SZ / 2; xx <= NH_SZ / 2; xx++ ) {
                    const float valA = iImageA.GetGreyLvl ( y + yy, x + xx ) - meanA;
                    const float valB = iImageB.GetGreyLvl ( y + yy, x + xx ) - meanB;

                    varA        += ( valA * valA );
                    varB        += ( valB * valB );
                    localScore  += ( valA * valB );
                }
            }
            varA /= (float)nPixels;
            varB /= (float)nPixels;
            localScore /= nPixels;

            if ( ( varA != 0.0f ) && ( varB != 0.0f ) ) {
                localScore /= sqrt ( varA * varB );
            } else {
                if ( ( varA != 0.0f ) || ( varB != 0.0f ) ) {
                    localScore = 0.0f;
                } else {
                    localScore = 1.f; 
                }
            }

            globalScore += ( 1.f - localScore ) / 2.0f;
            featureCount++;
        }
    }

    //std::cerr << featureCount << " " << zeroCount << " " << oneCount << std::endl;
    return ( globalScore / (float)featureCount );
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

float ImageBase::TemplateMatch (
    const ImageBase&        iMask,
    const int&              iSearchX,
    const int&              iSearchY,
    const int&              iSearchWidth,
    const int&              iSearchHeight,
    CartesianCoordinate&    oBestMatch,
    ImageBase*              oCorrelationMap
) const {
    Rectangle searchWindow (
        iSearchX,
        iSearchY,
        iSearchWidth,
        iSearchHeight
    );
    return TemplateMatch (
        iMask,
        searchWindow,
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
    if ( oCorrelationMap != NULL ) {
        if ( oCorrelationMap->GetHeight() != iSearchWindow.Height() ) {
            oCorrelationMap->SetHeight( iSearchWindow.Height() );
        }
        if ( oCorrelationMap->GetWidth() != iSearchWindow.Width() ) {
            oCorrelationMap->SetWidth( iSearchWindow.Width() );
        }
        //if ( oCorrelationMap->GetMaxGreyLevel() != GetMaxGreyLevel() ) {
            oCorrelationMap->SetMaxGreyLevel( 255 );
        //}
    }
    oBestMatch.x = oBestMatch.y = -1;

    float maskVar = 0;
    float maskMean = 0;
    unsigned int nPixels = iMask.GetWidth() * iMask.GetHeight();
    for ( int xx = 0; xx < iMask.GetWidth(); xx++ ) {
        for ( int yy = 0; yy < iMask.GetHeight(); yy++ ) {
            float maskVal = iMask.GetGreyLvl (
                yy,
                xx
            );
            
            maskMean += maskVal;
        }
    }
    maskMean /= (float)nPixels;

    for ( int xx = 0; xx < iMask.GetWidth(); xx++ ) {
        for ( int yy = 0; yy < iMask.GetHeight(); yy++ ) {
            float maskVal = iMask.GetGreyLvl (
                yy,
                xx
            ) - maskMean;
            
            maskVar += ( maskVal * maskVal );
        }
    }
    maskVar /= (float)nPixels;
    
    float bestMatchVal = -100;
    CartesianCoordinate maskCenter = iMask.Center();

    const int X0 = max ( iSearchWindow.X (), 0 );
    const int Y0 = max ( iSearchWindow.Y (), 0 );
    const int XE = min ( iSearchWindow.Right (), GetWidth  () );
    const int YE = min ( iSearchWindow.Bottom (), GetHeight () );

    #pragma omp parallel for
    for ( int x = X0; x < XE; x++ ) {
        #pragma omp parallel for
        for ( int y = Y0; y < YE; y++ ) {
            float val = 0;
            float myMean = 0;
            float myVar = 0;
            for ( int xx = 0; xx < iMask.GetWidth () ; xx++ ) {
                for ( int yy = 0; yy < iMask.GetHeight (); yy++ ) {
                    float myVal   = GetGreyLvl (
                        y + yy - maskCenter.y,
                        x + xx - maskCenter.x
                    );

                    myMean += myVal;
                }
            }
            myMean /= (float)nPixels; 

            for ( int xx = 0; xx < iMask.GetWidth () ; xx++ ) {
                for ( int yy = 0; yy < iMask.GetHeight (); yy++ ) {
                    float myVal   = GetGreyLvl (
                        y + yy - maskCenter.y,
                        x + xx - maskCenter.x
                    ) - myMean;
                    float maskVal = iMask.GetGreyLvl (
                        yy,
                        xx
                    ) - maskMean;

                    myVar += ( myVal * myVal );
                    val += ( myVal * maskVal );
                }
            }
            myVar /= (float)nPixels;
            val /= (float)nPixels;

            if ( ( myVar != 0.0f ) && ( maskVar != 0.0f ) ) {
                val /= sqrt ( myVar * maskVar );
            } else {
                if ( ( myVar != 0.0f ) || ( maskVar != 0.0f ) ) {
                    val = 0.0f;
                } else {
                    val = 1.0f;
                } 
            }

            if ( oCorrelationMap != NULL ) {
                oCorrelationMap->SetNormed (
                    y - iSearchWindow.Y(), 
                    x - iSearchWindow.X(),
                    val
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

    CartesianCoordinate topLeft ( 0, 0 );

    return PixelCorrelation ( iOther, topLeft, topLeft, GetWidth (), GetHeight () );
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
                    float leftVal  = correlationMapL.GetNormed ( j, i );
                    float rightVal = correlationMapR.GetNormed ( j, i );
    
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

float ImageBase::PixelCorrelation (
   const ImageBase& iOther,
   const CartesianCoordinate& p,
   const CartesianCoordinate& q,
   const int& WinWidth,
   const int& WinHeight
) const {
    float correlation   = 0.f;
    float myVar         = 0.f;
    float otherVar      = 0.f;
    float myMean        = 0.f;
    float otherMean     = 0.f;
    unsigned int nPixels = WinWidth * WinHeight;

    for (
        int x = -WinWidth / 2;
        x <= WinWidth / 2;
        x++
    ) {
        for (
            int y = -WinHeight / 2;
            y <= WinHeight / 2;
            y++
        ) {
            const float myVal       = GetGreyLvl (
                    p.y + y,
                    p.x + x
                    );
            const float otherVal    = iOther.GetGreyLvl (
                    q.y + y,
                    q.x + x
                    );

            myMean      += myVal;
            otherMean   += otherVal;
        } 
    }
    myMean      /= (float)nPixels;
    otherMean   /= (float)nPixels;

    for (
        int x = -WinWidth / 2;
        x <= WinWidth / 2;
        x++
    ) {
        for (
            int y = -WinHeight / 2;
            y <= WinHeight / 2;
            y++
        ) {
            float myVal    = GetGreyLvl ( p.y + y, p.x + x ) - myMean;
            float otherVal = iOther.GetGreyLvl ( q.y + y, q.x + x ) - otherMean;

            myVar       += ( myVal * myVal );
            otherVar    += ( otherVal * otherVal );
            correlation += ( myVal * otherVal );
        } 
    } 
    myVar /= (float)nPixels;
    otherVar /= (float)nPixels;
    correlation /= (float)nPixels;

    if ( ( myVar != 0.0f ) && ( otherVar != 0.0f ) ) {
        correlation /= sqrt ( myVar * otherVar );
    } else {
        if ( ( myVar != 0.0f ) || ( otherVar != 0.0f ) ) {
            correlation = 0.0f;
        } else {
            correlation = 1.0f;
        } 
    }

    if (correlation > 1.0f) {
        correlation = 1.0f;
    }
    if (correlation < -1.0f) {
        correlation = -1.0f;
    }

    return ( correlation );
}
