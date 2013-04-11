#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream

#define _USE_MATH_DEFINES
#include <cmath>
#include "Image.h"

inline bool IsOdd( const int iVal )
{
    return !(iVal & 0x1);
}
template<typename T>
inline bool InRange( 
    const T& iVal, 
    const T& iMin, 
    const T& iMax
) {
    return ( ( iVal >= iMin  ) && ( iVal <= iMax ) );
}

inline int abs( const int iVal )
{
    return (iVal < 0) ? -iVal : iVal;
}

template<typename T>
inline T min(T a, T b)
{
    return ( (a <= b) ? a : b );    
}
template<typename T>
inline T max(T a, T b)
{
    return ( (a >= b) ? a : b );    
}

Image::Image(const int iWidth, const int iHeight, const int iGreyLevel)
    : m_height( iHeight ), 
      m_width( iWidth ), 
      m_maxGreyLevel( iGreyLevel )
{
    m_figure.resize( m_height, m_width );
    m_normalisedFigure.resize( m_height, m_width );
}

Image::Image( const std::string& iFilename )
{
    LoadFromFile( iFilename );
}

void Image::LoadFromFile( const std::string& iFilename )
{
    int width = 0, height = 0, greyLevel = 0, i = 0, j = 0;
    int isBinary = 0;
    std::stringstream ss;
    std::string inputLine = "";

    /* Opening pgm file*/
    std::ifstream inFile( iFilename.c_str(), 
                            std::ifstream::in | std::ifstream::binary );

    /* First line : version of pgm file*/
    getline( inFile, inputLine );

    if ( inputLine.compare( "P2" ) != 0 && inputLine.compare( "P5" ) != 0 ) {
        std::cerr   << "Version error " 
                    << iFilename.c_str() 
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

    SetHeight( height );
    SetWidth( width );
    SetGreyLevel( greyLevel );

    /* resize matrix to receive the image */
    m_figure.resize( m_height, m_width );
    m_normalisedFigure.resize( m_height, m_width );

    if ( isBinary ) {
        for ( i = 0; i < height; i++ )
            for ( j = 0; j < width; j++ ) {
                m_figure( i, j ) = static_cast<int>( inFile.get() );
                m_normalisedFigure( i , j ) = (float)m_figure( i, j ) / (float)(greyLevel);
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
}

Image::~Image()
{
    // TODO Auto-generated destructor stub
}

void Image::CreateAsciiPgm( const std::string& iFilename )
{
    int i = 0, j = 0;
    std::ofstream ostr( iFilename.c_str() );

    /* writing header */
    ostr << "P2 " << std::endl;
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

void Image::SetGreyLvl( const int iRow, const int iCol, int iValue )
{
    if ( InRange( iCol, 0, m_width  - 1 ) &&
         InRange( iRow, 0, m_height - 1 ) ) {
        m_figure( iRow, iCol ) = iValue;
        m_normalisedFigure( iRow, iCol ) = (float)iValue / (float)m_maxGreyLevel;
    } else {
        throw BadIndex();
    }
}
void Image::SetNormed( const int iRow, const int iCol, float iValue )
{
    if ( InRange( iCol, 0, m_width  - 1 ) &&
         InRange( iRow, 0, m_height - 1 ) ) {
        m_figure( iRow, iCol ) = iValue * m_maxGreyLevel;
        m_normalisedFigure( iRow, iCol ) = iValue;
    } else {
        throw BadIndex();
    }
}
void Image::SetGreyLvl( const CartesianCoordinate& iPos, int iValue )
{
    SetGreyLvl( iPos.y, iPos.x, iValue );
}
void Image::SetNormed( const CartesianCoordinate& iPos, float iValue )
{
    SetNormed( iPos.y, iPos.x, iValue );
}
const int   Image::GetGreyLvl( const int iRow, const int iCol ) const
{
    int row = abs(iRow);
    int col = abs(iCol);
    row = IsOdd(row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd(col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);
    
    return m_figure( row, col );

    //if ( InRange(iRow, 0,  m_height - 1) && 
    //     InRange(iCol, 0,  m_width  - 1) ) {
    //    return m_figure( iRow, iCol );
    //} else {
    //    return 0;
    //}
}
const int   Image::GetGreyLvl( const CartesianCoordinate& iPos ) const
{
    return GetGreyLvl( iPos.y, iPos.x );
}
const float Image::GetNormed( const int iRow, const int iCol ) const
{
    int row = abs(iRow);
    int col = abs(iCol);
    row = IsOdd(row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd(col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);

    return m_normalisedFigure( row, col );

    //if ( InRange(iRow, 0,  m_height - 1) && 
    //     InRange(iCol, 0,  m_width  - 1) ) {
    //    return m_normalisedFigure( iRow, iCol );
    //} else {
    //    return 0.0f;
    //}
}
const float Image::GetNormed( const CartesianCoordinate& iPos ) const
{
    return GetNormed( iPos.y, iPos.x );
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

Image Image::SubImage(
    const int iX,
    const int iY,
    const int iWidth,
    const int iHeight
) const {
    Image subImage( iWidth, iHeight, m_maxGreyLevel );

    for ( int x = 0; x < iWidth; x++ ) {
        for ( int y = 0; y < iHeight; y++ ) {
            CartesianCoordinate subCoord(      x,      y );
            CartesianCoordinate imgCoord( iX + x, iY + y );
            
            subImage.SetGreyLvl( subCoord, GetGreyLvl( imgCoord ) );
        }
    }

    return subImage;
}

CartesianCoordinate Image::Center() const 
{
    return CartesianCoordinate( m_width / 2, m_height / 2 );
}

Image Image::FourierTransform() const
{
    Image transform( m_width, m_height, 65535 );

    float maxVal = 0;
    for ( int x = 0; x < m_width; x++ ) {
        for ( int y = 0; y < m_height; y++ ) {
            CartesianCoordinate transCoord( x, y );
            float ftVal = 0;
            for ( int xx = 0; xx < m_width; xx++ ) {
                for ( int yy = 0; yy < m_height; yy++ ) {
                    CartesianCoordinate myCoord( xx, yy );

                    float myVal = GetNormed( myCoord ); 
                    float arg = -2 * M_PI;
                    arg *= ( ( x * xx / m_width ) + ( y * yy / m_height ) );
                    
                    float re = myVal * cos(arg); 
                    float im = myVal * sin(arg); 
                    
                    ftVal += sqrt(re*re + im*im); 
                }
            }    
            transform.SetNormed( transCoord, ftVal ); 
            
            maxVal = max(ftVal, maxVal);
        }
    }    
    for ( int x = 0; x < m_width; x++ ) {
        for ( int y = 0; y < m_height; y++ ) {
            CartesianCoordinate c( x, y );

            transform.SetNormed( c, transform.GetNormed(c) / maxVal );
        }
    }
    transform.Recalculate();

    return transform;
}

Image Image::TemplateMatch(
    const Image&            iMask,
    CartesianCoordinate&    oBestMatch
) const {
    const Image& me = (*this);
    Image correlation( m_width, 
                       m_height, 
                       65535);

    int nMaskElems = iMask.m_width * iMask.m_width;
    
    float maskDenom = 0;
    for ( int xx = 0; xx < iMask.GetWidth(); xx++ ) {
        for ( int yy = 0; yy < iMask.GetHeight(); yy++ ) {
            CartesianCoordinate maskCoords( xx, yy );
            
            float maskVal = iMask.GetNormed( maskCoords );
            
            maskDenom += maskVal * maskVal; 
        }
    }

    float bestMatchVal = 0;
    CartesianCoordinate c( iMask.GetWidth() / 2, iMask.GetHeight() / 2);
    for ( int x = 0; x < correlation.GetWidth(); x++ ) {
        for ( int y = 0; y < correlation.GetHeight(); y++ ) {
            CartesianCoordinate corrCoords( x, y );

            float val = 0;
            float myDenom = 0;
            for ( int xx = -c.x; xx <= c.x ; xx++ ) {
                for ( int yy = -c.y; yy <= c.y; yy++ ) {
                    CartesianCoordinate myCoords( x + xx, y + yy );
                    CartesianCoordinate maskCoords( xx + c.x, yy + c.y );

                    float myVal   = me.GetNormed( myCoords );
                    float maskVal = iMask.GetNormed( maskCoords );

                    myDenom += myVal * myVal; 
                    
                    val += myVal * maskVal;
                }
            }
            val /= sqrt(maskDenom * myDenom);
            
            correlation.SetNormed( corrCoords, val);
            if ( val >= bestMatchVal ) {
                bestMatchVal = val;
                oBestMatch = corrCoords;
            }
        }
    }

    return correlation;
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

void Image::Recalculate()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            m_figure( i, j ) = m_normalisedFigure( i, j ) * m_maxGreyLevel;
        }
    }
}

void Image::SetHeight( const int iHeight )
{
    m_height = iHeight;
}

void Image::SetWidth( const int iWidth )
{
    m_width = iWidth;
}

void Image::SetGreyLevel( const int iGreyLevel )
{
    m_maxGreyLevel = iGreyLevel;
}

int const& Image::GetHeight() const
{
    return m_height;
}

int const& Image::GetWidth() const
{
    return m_width;
}

int const& Image::GetMaxGreyLevel() const
{
    return m_maxGreyLevel;
}

