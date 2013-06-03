#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream
#include "omp.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include "Image.h"
#include "Rectangle.h"
#include "SubImage.h"
#include "MathUtils.h"

#include "Config.h"
#include <iomanip>

Image::Image ()
    :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    m_figure = new Eigen::MatrixXi ();
    m_normalisedFigure = new Eigen::MatrixXf ();
    ResetMatrix ();
}
Image::Image (
    const int& iWidth,
    const int& iHeight,
    const int& iGreyLevel
)   :   m_height ( iHeight ), 
        m_width ( iWidth ), 
        m_maxGreyLevel ( iGreyLevel ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    m_figure = new Eigen::MatrixXi ();
    m_normalisedFigure = new Eigen::MatrixXf ();
    ResetMatrix ();
}
Image::Image (
    const std::string& iFilename
)   :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure ( 0x0  ),
        m_normalisedFigure ( 0x0 )
{
    LoadFromFile ( iFilename );
}
Image::~Image () {
    ClearMatrix ();
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

        ClearMatrix ();
        m_figure = new Eigen::MatrixXi ();
        m_normalisedFigure = new Eigen::MatrixXf ();

        /* Second line : comment */
        getline( inFile, inputLine );

        /* Third  line : size
           Fourth line : grey level*/
        inFile >> width >> height >> greyLevel ;

        //SetHeight( height );
        //SetWidth( width );
        SetDimensions ( width, height );
        SetMaxGreyLevel ( greyLevel );

        if ( isBinary ) {
            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    int readValue = static_cast<int>( inFile.get() );

                    SetGreyLvl ( i, j, readValue );
                }
            }
        } else {
            ss << inFile.rdbuf();

            for ( i = 0; i < height; i++ ) {
                for ( j = 0; j < width; j++ ) {
                    int readValue = 0;
                    ss >> readValue;

                    SetGreyLvl ( i, j, readValue );
                }
            }
        }
        inFile.close();
    } else {
        std::cerr << "File could not be opened" << std::endl; 
    }
}


void Image::RecalculateGreyLvl()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            (*m_figure) ( i, j ) = (*m_normalisedFigure) ( i, j ) * m_maxGreyLevel;
        }
    }
}
void Image::RecalculateNormalised()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            (*m_normalisedFigure) ( i, j ) = (float)(*m_figure) ( i, j ) / (float)m_maxGreyLevel;
        }
    }
}

inline void Image::ClearMatrix ()
{
    if ( m_figure ) {
        delete m_figure;
        m_figure = 0x0;
    }
    if ( m_normalisedFigure ) {
        delete m_normalisedFigure;
        m_normalisedFigure = 0x0;
    }
}

inline void Image::ResetMatrix ()
{
    m_figure->resize ( m_height, m_width );
    m_normalisedFigure->resize ( m_height, m_width );
}

inline void Image::SetDimensions (
    const int& iWidth,
    const int& iHeight
) {
    m_width = iWidth;
    m_height = iHeight;
    ResetMatrix ();
}

inline void Image::SetHeight (
    const int& iHeight
) {
    m_height = iHeight;
    ResetMatrix ();
}

inline void Image::SetWidth (
    const int& iWidth
) {
    m_width = iWidth;
    ResetMatrix ();
}

inline void Image::SetMaxGreyLevel (
    const int&  iGreyLevel,
    const bool& iRenormalise
) {
    m_maxGreyLevel = iGreyLevel;
    if ( iRenormalise ) {
        RecalculateNormalised ();
    }
}

inline int const& Image::GetHeight ()
const {
    return m_height;
}

inline int const& Image::GetWidth ()
const {
    return m_width;
}

inline int const& Image::GetMaxGreyLevel ()
const {
    return m_maxGreyLevel;
}

inline void Image::SetGreyLvl (
    const int& iRow,
    const int& iCol,
    const int& iValue
) {
    if (
            InRange ( iCol, 0, m_width  - 1 )
        &&  InRange ( iRow, 0, m_height - 1 )
    ) {
        (*m_figure) ( iRow, iCol ) = iValue;
        (*m_normalisedFigure) ( iRow, iCol ) = (float)iValue / (float)m_maxGreyLevel;
    } else {
        throw BadIndex( iCol, iRow );
    }
}

inline void Image::SetNormed (
    const int&      iRow,
    const int&      iCol,
    const float&    iValue
) {
    if (
            InRange ( iCol, 0, m_width  - 1 )
        &&  InRange ( iRow, 0, m_height - 1 )
    ) {
        (*m_figure) ( iRow, iCol ) = iValue * m_maxGreyLevel;
        (*m_normalisedFigure) ( iRow, iCol ) = iValue;
    } else {
        throw BadIndex ( iCol, iRow );
    }
}

inline void Image::SetGreyLvl (
    const CartesianCoordinate&  iPos,
    const int&                  iValue
) {
    SetGreyLvl ( iPos.y, iPos.x, iValue );
}

inline void Image::SetNormed (
    const CartesianCoordinate&  iPos,
    const float&                iValue
) {
    SetNormed ( iPos.y, iPos.x, iValue );
}

inline const int& Image::GetGreyLvl (
    const int& iRow,
    const int& iCol
) const {
    int row = abs(iRow);
    int col = abs(iCol);

    row = IsOdd (row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd (col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);
    
    return (*m_figure) ( row, col );
}

inline const int& Image::GetGreyLvl (
    const CartesianCoordinate& iPos
) const {
    return GetGreyLvl ( iPos.y, iPos.x );
}

inline const float& Image::GetNormed (
    const int& iRow,
    const int& iCol
) const {
    int row = abs(iRow);
    int col = abs(iCol);
    
    row = IsOdd(row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd(col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);

    return (*m_normalisedFigure) ( row, col );
}

inline const float& Image::GetNormed (
    const CartesianCoordinate& iPos
) const {
    return GetNormed ( iPos.y, iPos.x );
}

