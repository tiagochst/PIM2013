#include <omp.h>
#include <iostream> // cout, cerr
#include <fstream> // ifstream
#include <sstream> // stringstream

#define _USE_MATH_DEFINES
#include <cmath>
#include "Image.h"
#include "Rectangle.h"
#include "SubImage.h"
#include "MathUtils.h"
#include "Tools.h"

#include "Config.h"
#include <iomanip>

Image::Image ()
    :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure (),
        m_normalisedFigure ()
{
    ResetMatrix ();
}
Image::Image (
    const int& iWidth,
    const int& iHeight,
    const int& iGreyLevel
)   :   m_height ( iHeight ), 
        m_width ( iWidth ), 
        m_maxGreyLevel ( iGreyLevel ),
        m_figure (),
        m_normalisedFigure ()
{
    ResetMatrix ();
}
Image::Image (
    const std::string& iFilename
)   :   m_height ( 1 ), 
        m_width ( 1 ), 
        m_maxGreyLevel ( 1 ),
        m_figure (),
        m_normalisedFigure ()
{
    LoadFromFile ( iFilename );
}

Image::~Image () {
}

void Image::LoadFromFile (
    const std::string& iFilename
) {
    int width = 0, height = 0, greyLevel = 0;
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

        bool widthRead      = true;
        bool heightRead     = true;
        bool maxValueRead   = true;
        while ( widthRead || heightRead || maxValueRead ) {
            bool isComment = false;
            std::string inputLine;
            getline (
                inFile,
                inputLine
            );
            std::stringstream lineStream ( inputLine );
            //std::cout << "line read: " << inputLine << std::endl;

            while (
                    !lineStream.eof ()
                &&  ( widthRead || heightRead || maxValueRead )
            ) {
                char peeked = lineStream.peek ();
                while ( peeked == '#' || peeked == ' ' ) {
                    //std::cout << "blank char '" << peeked << "'" << std::endl;
                    if ( peeked = '#' ) {
                        isComment = true;
                        break;
                    }
                    lineStream >> peeked;
                    peeked = lineStream.peek ();
                }
                if ( isComment ) {
                    isComment = false;
                    break;
                }
                if ( widthRead && !lineStream.eof () ) {
                    lineStream >> width;
                    //std::cout << "w: " << width << std::endl;
                    widthRead = false;
                }
                if ( heightRead && !lineStream.eof () ) {
                    lineStream >> height;
                    //std::cout << "h: " << height << std::endl;
                    heightRead = false;
                }
                if ( maxValueRead && !lineStream.eof () ) {
                    lineStream >> greyLevel;
                    //std::cout << "m: " << greyLevel << std::endl;
                    maxValueRead = false;
                }
            }
        }
        SetMaxGreyLevel ( greyLevel );
        SetDimensions ( width, height );

        if ( !isBinary ) {
            ss << inFile.rdbuf();
        }
        for ( int i = 0; i < height; i++ ) {
            for ( int j = 0; j < width; j++ ) {
                int readValue = 0;

                if ( isBinary ) {
                    readValue = static_cast<int>( inFile.get() );
                } else {
                    ss >> readValue;
                }

                SetGreyLvl ( i, j, readValue );
            }
        }
        inFile.close();
    } else {
        std::cout << iFilename << std::endl;
        std::cerr << ": File could not be opened" << std::endl; 
    }
}


void Image::RecalculateGreyLvl()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            m_figure ( i, j ) = m_normalisedFigure ( i, j ) * m_maxGreyLevel;
        }
    }
}
void Image::RecalculateNormalised()
{
    for ( int i = 0; i < m_height; i++ ) {
        for ( int j = 0; j < m_width; j++ ) {
            m_normalisedFigure ( i, j ) = (float)m_figure ( i, j ) / (float)m_maxGreyLevel;
        }
    }
}

inline void Image::ClearMatrix ()
{
    m_figure.fill ( 0 );
    m_normalisedFigure.fill ( 0.0f );
}

inline void Image::ResetMatrix ()
{
    m_figure.resize ( m_height, m_width );
    m_normalisedFigure.resize ( m_height, m_width );
    ClearMatrix ();
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
        m_figure ( iRow, iCol ) = iValue;
        m_normalisedFigure ( iRow, iCol ) = (float)iValue / (float)m_maxGreyLevel;
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
        m_figure ( iRow, iCol ) = iValue * m_maxGreyLevel;
        m_normalisedFigure ( iRow, iCol ) = iValue;
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
    //if (
    //    iRow < 0 || iRow >= m_height ||
    //    iCol < 0 || iCol >= m_width
    //) {
    //    return 0;
    //}
    int row = abs(iRow);
    int col = abs(iCol);

    row = IsOdd (row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd (col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);
    
    return m_figure ( row, col );
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
    //if (
    //    iRow < 0 || iRow >= m_height ||
    //    iCol < 0 || iCol >= m_width
    //) {
    //    return 0.0f;
    //}
    int row = abs(iRow);
    int col = abs(iCol);
    
    row = IsOdd(row / m_height) ? (row % m_height) : (m_height - (row % m_height) - 1);
    col = IsOdd(col / m_width ) ? (col % m_width ) : (m_width  - (col % m_width ) - 1);

    return m_normalisedFigure ( row, col );
}

inline const float& Image::GetNormed (
    const CartesianCoordinate& iPos
) const {
    return GetNormed ( iPos.y, iPos.x );
}

static const int GaussianResolution = 5;
static const float GaussianCoefficients[5][5] = {
    {  1.0f/273.0f,  4.0f/273.0f,  7.0f/273.0f,  4.0f/273.0f,  1.0f/273.0f },
    {  4.0f/273.0f, 16.0f/273.0f, 26.0f/273.0f, 16.0f/273.0f,  4.0f/273.0f },
    {  7.0f/273.0f, 26.0f/273.0f, 41.0f/273.0f, 26.0f/273.0f,  7.0f/273.0f },
    {  4.0f/273.0f, 16.0f/273.0f, 26.0f/273.0f, 16.0f/273.0f,  4.0f/273.0f },
    {  1.0f/273.0f,  4.0f/273.0f,  7.0f/273.0f,  4.0f/273.0f,  1.0f/273.0f }
};
//static const int GaussianResolution = 3;
//static const float GaussianCoefficients[3][3] = {
//    { 0.07511360795411207f, 0.12384140315297386f, 0.07511360795411207f }, 
//    { 0.12384140315297386f, 0.20417995557165622f, 0.12384140315297386f }, 
//    { 0.07511360795411207f, 0.12384140315297386f, 0.07511360795411207f } 
//};
Image* Image::Filter () {
    Image* result = new Image ( GetWidth (), GetHeight (), GetMaxGreyLevel () );

    for ( unsigned int i = 0; i < GetWidth (); i++ ) {
        for ( unsigned int j = 0; j < GetHeight (); j++ ) {
            float filteredValue = 0;
            
            for ( int ii = 0; ii < GaussianResolution; ii++ ) {
                for ( int jj = 0; jj < GaussianResolution; jj++ ) {
                    const float& value = GetGreyLvl (
                        j + jj - GaussianResolution / 2,
                        i + ii - GaussianResolution / 2
                    );

                    filteredValue   += GaussianCoefficients[jj][ii]
                                     * value;

                }
            }

            result->SetGreyLvl ( j, i, filteredValue );
        }
    }

    return result;
}

Image* Image::SubSample (
    const unsigned int&         iFactor
) const {
    Image* result = new Image ( GetWidth () / iFactor, GetHeight () / iFactor, GetMaxGreyLevel () );

    for ( unsigned int i = 0; ( i * iFactor ) < GetWidth (); i++ ) {
        for ( unsigned int j = 0; ( j * iFactor ) < GetHeight (); j++ ) {
            result->SetGreyLvl (
                j,
                i,
                GetGreyLvl ( iFactor * j, iFactor * i )
            );
        }
    }

    return result;
}

void Image::CalculateAnchors (
    const unsigned int&         iTotalFrameCount,
    const unsigned int&         iReferenceFrame,
    const std::string&          iSearchPath,
    const std::string&          iImagePrefix,
    std::vector<unsigned int>&  oAnchorList
) {
    std::string localImagePrefix = iSearchPath + iImagePrefix;

    Image   refImage ( localImagePrefix + toString(iReferenceFrame) + ".pgm");
    Image*  curImage = (Image*)0x0;

    omp_lock_t curImgLock;
    omp_init_lock ( &curImgLock );

    unsigned int curImgIdx  = 0;
    #pragma omp parallel sections shared ( curImage ) 
    {
        #pragma omp section
        {
            while (
                curImgIdx < iTotalFrameCount
            ) {
                if (
                    curImgIdx == iReferenceFrame
                ) {
                    omp_set_lock ( &curImgLock );

                    curImgIdx++;

                    omp_unset_lock ( &curImgLock );

                    continue;
                }

                Image* newImage = new Image ( localImagePrefix + toString(curImgIdx) + ".pgm");

                omp_set_lock ( &curImgLock );

                curImage = newImage;
                curImgIdx++;

                omp_unset_lock ( &curImgLock );
            }
        }

        #pragma omp section
        {
            while (
                curImgIdx <= iTotalFrameCount
            ) {
                omp_set_lock ( &curImgLock );

                if (
                    curImage != (Image*)0x0
                ) {
                    const float score = ImageBase::CalculateErrorScore ( refImage, *curImage );
                    if (
                        score <= 1
                    ) {
                        oAnchorList.push_back ( curImgIdx - 1 );
                    }

                    delete curImage;
                    curImage = (Image*)0x0;
                }

                omp_unset_lock ( &curImgLock );
                
                if (
                    curImgIdx == iTotalFrameCount
                ) {
                    break;
                }
            }
        }
    }
    omp_destroy_lock ( &curImgLock );
}

