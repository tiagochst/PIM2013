#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream
#include "omp.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include "Image.h"

#include "Config.h"
#include <iomanip>

Image::Image ()
    : m_height ( 1 ), 
      m_width ( 1 ), 
      m_maxGreyLevel ( 1 )
{
}

Image::Image (
    const int& iWidth,
    const int& iHeight,
    const int& iGreyLevel
)   :   m_height ( iHeight ), 
        m_width ( iWidth ), 
        m_maxGreyLevel ( iGreyLevel )
{
    ResetMatrix ();
}

Image::Image (
    const std::string& iFilename
)   :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 0 )
{
    LoadFromFile ( iFilename );
}

void Image::LoadFromFile (
    const std::string& iFilename
) {
    int width = 0, height = 0, greyLevel = 0, i = 0, j = 0;
    int isBinary = 0;
    std::stringstream ss;
    std::string inputLine = "";

    /* Opening pgm file*/
    std::ifstream inFile (
        iFilename.c_str (), 
        std::ifstream::in | std::ifstream::binary
    );

    if (inFile.is_open() && inFile.good()) {
        /* First line : version of pgm file*/
        getline( inFile, inputLine );

        if (
                inputLine.compare( "P2" ) != 0 
            &&  inputLine.compare( "P5" ) != 0
        ) {
            std::cerr   << "Version error " 
                        << iFilename.c_str()
                        << " Version: "
                        << inputLine 
                        << std::endl;

        } 
        if ( inputLine.compare( "P5" ) == 0 ) {
            isBinary = 1;
        }

        /* Second line : comment */
        getline( inFile, inputLine );

        /* Third  line : size
           Fourth line : grey level*/
        inFile >> width >> height >> greyLevel ;

        //SetHeight( height );
        //SetWidth( width );
        SetDimensions ( width, height );

        if ( isBinary ) {
            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    m_figure( i, j ) = static_cast<int>( inFile.get() );
                    m_normalisedFigure( i , j ) = (float)m_figure( i, j ) / (float)(greyLevel);
                }
            }
        } else {
            ss << inFile.rdbuf();

            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    ss >> m_figure( i, j );
                    m_normalisedFigure( i , j ) = (float)m_figure( i, j ) / (float)(greyLevel);
                }
            }
        }
        inFile.close();
        m_maxGreyLevel = greyLevel;
    } else {
        std::cerr << "File could not be opened" << std::endl; 
    }
}

Image::~Image()
{
}

void Image::CreateAsciiPgm( const std::string& iFilename )
{
    int i = 0, j = 0;
    std::ofstream ostr( iFilename.c_str() );

    /* writing header */
    ostr << "P2" << std::endl;
    ostr << "# CREATOR: PIM380 PROJ" << std::endl;;
    ostr << m_width << " " << m_height << std::endl;
    ostr << m_maxGreyLevel << std::endl;

    for ( i = 0; i < m_height; i++ ) {
        for ( j = 0; j < m_width; j++ ) {
            ostr << m_figure( i, j ) << std::endl;
        }
    }

    ostr.close();
}

float Image::Correlation( const Image& iOther ) const
{
    if ( iOther.GetWidth() != m_width || 
            iOther.GetHeight() != m_height  ) {
        throw IncompatibleImages();
    }

    float correlation = 0;
    for ( int x = 0; x < m_width; x++ ) {
        for ( int y = 0; y < m_height; y++ ) {
            CartesianCoordinate c( x, y );

            correlation += GetNormed( c ) * iOther.GetNormed( c );
        } 
    } 
    
    return correlation;
}

void Image::SubImage (
    const Rectangle&    iRegion,
    Image&              oSubImage
) const {
    SubImage (
        iRegion.X (),
        iRegion.Y (),
        iRegion.Width (),
        iRegion.Height (),
        oSubImage
    );
}

void Image::SubImage (
    const int&  iX,
    const int&  iY,
    const int&  iWidth,
    const int&  iHeight,
    Image&      oSubImage
) const {
    if ( oSubImage.GetHeight () != iHeight ) {
        oSubImage.SetHeight ( iHeight );
    }
    if ( oSubImage.GetWidth () != iWidth ) {
        oSubImage.SetWidth ( iWidth );
    }
    if ( oSubImage.GetMaxGreyLevel () != m_maxGreyLevel ) {
        oSubImage.SetMaxGreyLevel ( m_maxGreyLevel );
    }

    for ( int x = 0; x < iWidth; x++ ) {
        for ( int y = 0; y < iHeight; y++ ) {
            CartesianCoordinate subCoord (      x,      y );
            CartesianCoordinate imgCoord ( iX + x, iY + y );
            
            oSubImage.SetGreyLvl ( subCoord, GetGreyLvl ( imgCoord ) );
        }
    }
}

Image Image::FourierTransform () const
{
    Image transform ( m_width, m_height, 65535 );

    float maxVal = 0;
    for ( int x = 0; x < m_width; x++ ) {
        for ( int y = 0; y < m_height; y++ ) {
            CartesianCoordinate transCoord ( x, y );

            float ftVal = 0;
            
            for ( int xx = 0; xx < m_width; xx++ ) {
                for ( int yy = 0; yy < m_height; yy++ ) {
                    CartesianCoordinate myCoord ( xx, yy );

                    float myVal = GetNormed ( myCoord ); 
                    float arg = -2 * M_PI;
                    arg *= ( ( x * xx / m_width ) + ( y * yy / m_height ) );
                    
                    float re = myVal * cos ( arg ); 
                    float im = myVal * sin ( arg ); 
                    
                    ftVal += sqrt ( re * re + im * im ); 
                }
            }    
            transform.SetNormed ( transCoord, ftVal ); 
            
            maxVal = max ( ftVal, maxVal );
        }
    }    
    for (
        int x = 0;
        x < m_width;
        x++
    ) {
        for (
            int y = 0;
            y < m_height;
            y++
        ) {
            CartesianCoordinate c ( x, y );

            transform.SetNormed (
                c,
                transform.GetNormed ( c ) / maxVal
            );
        }
    }
    transform.RecalculateGreyLvl ();

    return transform;
}

float Image::TemplateMatch (
    const Image&            iMask,
    CartesianCoordinate&    oBestMatch,
    Image*                  oCorrelationMap
) const {
    Rectangle window ( 0, 0, m_width, m_height );
    TemplateMatch (
        iMask,
        window,
        oBestMatch,
        oCorrelationMap
    );
}

float Image::TemplateMatch(
    const Image&            iMask,
    const Rectangle&        iSearchWindow,
    CartesianCoordinate&    oBestMatch,
    Image*                  oCorrelationMap
) const {
    const Image& me = (*this);
    const Rectangle& sw( iSearchWindow );

    if ( oCorrelationMap != NULL ) {
        if ( oCorrelationMap->GetHeight() != sw.Height() ) {
            oCorrelationMap->SetHeight( sw.Height() );
        }
        if ( oCorrelationMap->GetWidth() != sw.Width() ) {
            oCorrelationMap->SetWidth( sw.Width() );
        }
        if ( oCorrelationMap->GetMaxGreyLevel() != m_maxGreyLevel ) {
            oCorrelationMap->SetMaxGreyLevel( m_maxGreyLevel );
        }
    }

    int nMaskElems = iMask.m_width * iMask.m_width;
    
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
                    val += myVal * maskVal;
                }
            }
            val /= sqrt(maskDenom * myDenom);
            
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

void Image::TrackPixels(
    const Image&        iRefImage,
    const Image&        iTargetImage,
    const int&          iWindowWidth,
    const int&          iWindowHeight,
    const int&          iNeighbourhoodWidth,
    const int&          iNeighbourhoodHeight,
    Image&              oDisplacementMapX,
    Image&              oDisplacementMapY
) {
    if ( iRefImage.GetHeight() != iTargetImage.GetHeight() ||
         iRefImage.GetWidth()  != iTargetImage.GetWidth() ) {
        throw IncompatibleImages();
    }

    int minValX = 0, minValY = 0;
    int maxValX = 0, maxValY = 0;
    #pragma omp parallel for
    for ( int x = 0; x < iRefImage.GetWidth(); x++ ) {
        #pragma omp parallel for 
        for ( int y = 0; y < iRefImage.GetHeight(); y++ ) {
            CartesianCoordinate bestMatchL;
            Image neighbourhoodL ( iNeighbourhoodWidth, iNeighbourhoodHeight, 65535 );
            Image correlationMapL ( iWindowWidth, iWindowHeight, 65535 );

            CartesianCoordinate bestMatchR;
            Image neighbourhoodR ( iNeighbourhoodWidth, iNeighbourhoodHeight, 65535 );
            Image correlationMapR ( iWindowWidth, iWindowHeight, 65535 );

            CartesianCoordinate nCenter = neighbourhoodL.Center();
            CartesianCoordinate wCenter ( iWindowWidth / 2, iWindowHeight / 2 );

            Rectangle nRegionL( x - nCenter.x,
                                y - nCenter.y, 
                                iNeighbourhoodWidth,
                                iNeighbourhoodHeight );
            Rectangle nRegionR( x - nCenter.x + 1,
                                y - nCenter.y, 
                                iNeighbourhoodWidth,
                                iNeighbourhoodHeight );
            Rectangle wRegionL( x - wCenter.x,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );
            Rectangle wRegionR( x - wCenter.x + 1,
                                y - wCenter.y,
                                iWindowWidth,
                                iWindowHeight );
            iRefImage.SubImage( nRegionL, neighbourhoodL );
            iRefImage.SubImage( nRegionR, neighbourhoodR );

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

            int valX = 10 * (bestMatch.x - x);
            int valY = 10 * (bestMatch.y - y);

            
            oDisplacementMapX.SetGreyLvl( y, x, valX );
            oDisplacementMapY.SetGreyLvl( y, x, valY );

            #pragma omp critical
            {
                minValX = min( valX, minValX );
                maxValX = max( valX, maxValX );
                minValY = min( valY, minValY );
                maxValY = max( valY, maxValY );
            }
        }
    }
    maxValX += abs(minValX);
    maxValY += abs(minValY);
    oDisplacementMapX.SetMaxGreyLevel(maxValX);
    oDisplacementMapY.SetMaxGreyLevel(maxValY);
    #pragma omp parallel for
    for ( int x = 0; x < iRefImage.GetWidth(); x++ ) {
        #pragma omp parallel for
        for ( int y = 0; y < iRefImage.GetHeight(); y++ ) {
            int valX = oDisplacementMapX.GetGreyLvl( y, x );
            int valY = oDisplacementMapY.GetGreyLvl( y, x );

            oDisplacementMapX.SetGreyLvl( y, x, ( valX + abs(minValX) ) );
            oDisplacementMapY.SetGreyLvl( y, x, ( valY + abs(minValY) ) );
        }
    }
}

Image Image::Difference( const Image& iOther ) const
{
    const Image& me = (*this);
    Image difference( m_width, m_height, m_maxGreyLevel );

    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            int val = me.GetGreyLvl( i, j ) - iOther.GetGreyLvl( i, j );
            difference.SetGreyLvl( i, j, abs( val ) );
        }
    }
    
    return difference;
}

void Image::RecalculateGreyLvl()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            m_figure( i, j ) = m_normalisedFigure( i, j ) * m_maxGreyLevel;
        }
    }
}
void Image::RecalculateNormalised()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            m_normalisedFigure( i, j ) = (float)m_figure( i, j ) / (float)m_maxGreyLevel;
        }
    }
}


