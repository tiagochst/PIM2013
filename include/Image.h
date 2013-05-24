#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"
#include "MathUtils.h"

class BadIndex {
    std::string m_what;
public:
    inline BadIndex(int iCol, int iRow)
    {
        std::stringstream ss;
        ss << "Col: " << iCol << " Row: " << iRow;
        m_what = ss.str();
    }
    inline const std::string& what() { return m_what; }
};
class IncompatibleImages {};

class Image {
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi     m_figure;
    Eigen::MatrixXf     m_normalisedFigure;

public:
    Image (
        const int& iWidth,
        const int& iHeight,
        const int& iGreyLevel
    );
    Image (
        const std::string& iFilename
    );
    virtual ~Image();

    void SetDimensions (
        const int& iWidth,
        const int& iHeight
    );
    void SetHeight (
        const int& iHeight
    );
    void SetWidth (
        const int& iWidth
    );
    void SetMaxGreyLevel (
        const int& iGreyLevel
    ); 

    int const& GetHeight () const;
    int const& GetWidth () const;
    int const& GetMaxGreyLevel () const;

    /* Read a binary (P5) or Ascii(P2) .pgm file*/
    void LoadFromFile (
        const std::string& iFilename
    );

    /* Verify if image was read correctly*/
    void CreateAsciiPgm (
        const std::string& iFilename
    );

    void SetGreyLvl (
        const int& iRow,
        const int& iCol,
        const int& iValue
    );
    void SetNormed (
        const int& iRow,
        const int& iCol,
        const float& iValue
    );
    void SetGreyLvl (
        const CartesianCoordinate&  iPos,
        const int&                  iValue
    );
    void SetNormed (
        const CartesianCoordinate&  iPos,
        const float&                iValue
    );

    const int& GetGreyLvl (
        const int& iRow,
        const int& iCol
    ) const;
    const int& GetGreyLvl (
        const CartesianCoordinate& iPos
    ) const;
    const float& GetNormed (
        const int& iRow,
        const int& iCol
    ) const;
    const float& GetNormed (
        const CartesianCoordinate& iPos
    ) const;

    CartesianCoordinate Center () const;
    
    float TemplateMatch (
        const Image&            iMask,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;
    float TemplateMatch (
        const Image&            iMask,
        const Rectangle&        iSearchWindow,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;

    void SubImage (
        const Rectangle&    iRegion,
        Image&              oSubImage
    ) const;
    void SubImage (
        const int&  iX,
        const int&  iY,
        const int&  iWidth,
        const int&  iHeight,
        Image&      oSubImage
    ) const;

    Image FourierTransform () const;
    
    Image Difference (
        const Image& iOther
    ) const;
    
    float Correlation (
        const Image& iOther
    ) const;

    static void TrackPixels(
        const Image&        iRefImage,
        const Image&        iTargetImage,
        const int&          iWindowWidth,
        const int&          iWindowHeight,
        const int&          iNeighbourhoodWidth,
        const int&          iNeighbourhoodHeight,
        Image&              oDisplacementMapX,
        Image&              oDisplacementMapY
    );

private:
    void ResetMatrix ();
    void RecalculateGreyLvl ();
    void RecalculateNormalised ();
};

inline CartesianCoordinate Image::Center () const 
{
    return CartesianCoordinate ( m_width / 2, m_height / 2 );
}

inline void Image::ResetMatrix ()
{
    m_figure.resize ( m_height, m_width );
    m_normalisedFigure.resize ( m_height, m_width );
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
    const int& iGreyLevel
) {
    m_maxGreyLevel = iGreyLevel;
    RecalculateNormalised ();
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

#endif /* IMAGE_H_ */
