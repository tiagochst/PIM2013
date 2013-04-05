#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream
#include "Image.h"

inline bool InRange( 
    const int& iVal, 
    const int& iMin, 
    const int& iMax
) {
    return ( ( iVal >= iMin  ) && ( iVal <= iMax ) );
}

inline int abs(int iVal) {
    return (iVal < 0) ? -iVal : iVal;
}

inline int max(int a, int b) {
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

    } else {
        std::cout << "Version : " << inputLine << std::endl;
    }

    if ( inputLine.compare( "P5" ) == 0 ) {
        isBinary = 1;
    }

    /* Second line : comment */
    getline( inFile, inputLine );
    std::cout << "Comment : " << inputLine << std::endl;

    /* Third  line : size
       Fourth line : grey level*/
    inFile >> height >> width >> greyLevel ;

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

        for ( i = 0; i < height; i++ )
            for ( j = 0; j < width; j++ ) {
                ss >> m_figure( i, j );
                m_normalisedFigure( i , j ) = (float)m_figure( i, j ) / (float)(greyLevel);
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
    ostr << m_height << " " << m_width << std::endl;
    ostr << m_maxGreyLevel << std::endl;

    for ( i = 0; i < m_height; i++ )
        for ( j = 0; j < m_width; j++ ) {
            ostr << m_figure( i, j ) << std::endl;
        }

    ostr.close();
}

float& Image::operator()( const int iRow, const int iCol )
{
    if ( InRange( iCol, 0, m_width  - 1 ) &&
         InRange( iRow, 0, m_height - 1 ) ) {

        return m_normalisedFigure(iRow, iCol);

    }
    throw BadIndex();
}
float Image::operator()( const int iRow, const int iCol ) const
{
    if ( InRange( iCol, 0, m_width  - 1 ) &&
         InRange( iRow, 0, m_height - 1 ) ) {

        return m_normalisedFigure(iRow, iCol);

    } else {

        return 0.0f;

    }
}

int Image::Correlation( const Image& iOther ) const
{
}

Image Image::PatternSearch( const Image& iMask, Coordinate& oBestMatch ) const
{
    const Image& me = (*this);
    Image correlation( m_width - iMask.GetWidth() + 1, 
                        m_height - iMask.GetHeight() + 1, 
                        65535);

    int bestMatchVal = 0;
    int minVal = 0;
    int nMaskElems = iMask.m_width * iMask.m_width;
    
    float maskDenom = 0;
    for ( int i = 0; i < iMask.m_height; i++ ) {
        for ( int j = 0; j < iMask.m_width; j++ ) {
            maskDenom += iMask( i, j ) * iMask( i, j );
        }
    }

    for ( int i = 0; i < correlation.GetHeight(); i++ ) {
        for ( int j = 0; j < correlation.GetWidth(); j++ ) {

            float val = 0;
            float myDenom = 0;
            for ( int k = 0; k < iMask.GetHeight(); k++ ) {
                for ( int l = 0; l < iMask.GetWidth(); l++ ) {
                    float myVal   = me( i + k, j + l );
                    float maskVal = iMask( k, l );

                    myDenom += myVal * myVal; 
                    
                    val += myVal * maskVal;
                }
            }
            val /= sqrt(maskDenom * myDenom);
            std::cout << val << " ";
            
            correlation(i, j) = val;
            if ( val > bestMatchVal ) {
                bestMatchVal = val;
                oBestMatch.x = i;
                oBestMatch.y = j; 
            }
        }
    }
    correlation.Recalculate();    

    return correlation;
}

Image Image::Difference( const Image& iOther ) const
{
    const Image& me = (*this);
    Image difference( m_width, m_height, m_maxGreyLevel );

    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            difference( i, j ) = abs( me( i, j ) - iOther( i, j ) );
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
