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

Image::Image(const int iWidth, const int iHeight, const int iGreyLevel)
    : m_height( iHeight ), 
      m_width( iWidth ), 
      m_maxGreyLevel( iGreyLevel )
{
    m_figure.resize( m_height, m_width );
}

inline int abs(int iVal) {
    return (iVal < 0) ? -iVal : iVal;
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

    /* path to our images */
    //std::string path = "./resources/Project/Images/";

    /* Opening pgm file*/
    std::ifstream inFile( (iFilename).c_str(), 
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

    /* Third line : size
       Forth line : grey level*/
    inFile >> height >> width >> greyLevel ;

    SetHeight( height );
    SetWidth( width );
    SetGreyLevel( greyLevel );

    /* resize matrix to receive the image */
    m_figure.resize( m_height, m_width );

    if ( isBinary ) {
        for ( i = 0; i < height; i++ )
            for ( j = 0; j < width; j++ ) {
                m_figure( i, j ) = static_cast<int>( inFile.get() );
            }

    } else {
        ss << inFile.rdbuf();

        for ( i = 0; i < height; i++ )
            for ( j = 0; j < width; j++ ) {
                ss >> m_figure( i, j );
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
    std::string prefix = "PROG";
    std::ofstream ostr( ( prefix + iFilename ).c_str() );

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

int& Image::operator()( const int iRow, const int iCol )
{
    if ( InRange( iCol, 0, m_width - 1) &&
         InRange( iRow, 0, m_height - 1) ) {

        return m_figure(iRow, iCol);

    }
    throw BadIndex();
}
int Image::operator()( const int iRow, const int iCol ) const
{
    if ( InRange( iCol, 0, m_width - 1) &&
         InRange( iRow, 0, m_height - 1) ) {

        return m_figure(iRow, iCol);

    } else {

        return 0;

    }
}

Image Image::Correlation( const Image& iOther ) const
{
    const Image& me = (*this);
    Image correlation(m_width, m_height, m_maxGreyLevel);

    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            int val = 0;
            int myMean = 0;
            int otherMean = 0;
            
            for ( int k = -4; k < 5; k++ ) {
                for ( int l = -4; l < 5; l++ ) {
                    myMean    += me(i + k, j + l); 
                    otherMean += iOther(i + k, j + l);
                }
            }
            myMean /= 81;
            otherMean /= 81;

            for ( int k = -4; k < 5; k++ ) {
                for ( int l = -4; l < 5; l++ ) {
                    val += (me(i + k, j + l) - myMean) * (iOther(i + k, j + l) - otherMean);
                }
            }
            
            correlation(i, j) = sqrt(val) / 81;
        }
    }
    
    return correlation;
}

Image Image::Difference( const Image& iOther ) const
{
    const Image& me = (*this);
    Image difference(m_width, m_height, m_maxGreyLevel);

    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            difference(i, j) = abs(me(i, j) - iOther(i, j));
        }
    }
    
    return difference;
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

int Image::GetHeight()
{
    return m_height;
}

int Image::GetWidth()
{
    return m_width;
}

int Image::GetMaxGreyLevel()
{
    return m_maxGreyLevel;
}

