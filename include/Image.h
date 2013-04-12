#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

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

struct CartesianCoordinate {
    int x;
    int y;
    inline CartesianCoordinate()
        : x( 0 ), y ( 0 )
    {}
    inline CartesianCoordinate( const int& x, const int& y )
        : x( x ), y ( y )
    {}
};
class Rectangle {
private:
    CartesianCoordinate     m_position;
    CartesianCoordinate     m_center;
    int                     m_right;
    int                     m_bottom;
    int                     m_width;
    int                     m_height;

public:
    inline Rectangle( const int& iX, const int& iY, const int& iWidth, const int& iHeight )
        : m_position( iX, iY ), m_center( iX + iWidth / 2, iY + iHeight / 2 ),
            m_width( iWidth ), m_height( iHeight ),
            m_right( iX + iWidth ), m_bottom( iY + iHeight )
    {}
    inline const int& Width() const { return m_width; }
    inline const int& Height() const { return m_height; } 
    inline const int& X() const { return m_position.x; }
    inline const int& Y() const { return m_position.y; }
    inline const int& Right() const { return m_right; }
    inline const int& Left() const { return m_position.x; }
    inline const int& Top() const { return m_position.y; }
    inline const int& Bottom() const { return m_bottom; }
    inline const CartesianCoordinate& Position() const { return m_position; }
    inline const CartesianCoordinate& Center() const { return m_center; }
};

class Image {
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi     m_figure;
    Eigen::MatrixXf     m_normalisedFigure;

public:
    Image( const int& iWidth, const int& iHeight, const int& iGreyLevel );
    Image( const std::string& iFilename );
    virtual ~Image();

    void SetHeight( const int& iHeight );
    void SetWidth( const int& iWidth );
    void SetMaxGreyLevel( const int& iGreyLevel ); 

    int const& GetHeight() const;
    int const& GetWidth() const;
    int const& GetMaxGreyLevel() const;

    /* Read a binary (P5) or Ascii(P2) .pgm file*/
    void LoadFromFile( const std::string& iFilename );

    /* Verify if image was read correctly*/
    void CreateAsciiPgm( const std::string& iFilename );

    void SetGreyLvl( const int& iRow, const int& iCol, const int& iValue );
    void SetNormed( const int& iRow, const int& iCol, const float& iValue );
    void SetGreyLvl( const CartesianCoordinate& iPos, const int& iValue );
    void SetNormed( const CartesianCoordinate& iPos, const float& iValue );

    const int&   GetGreyLvl( const int& iRow, const int& iCol ) const;
    const int&   GetGreyLvl( const CartesianCoordinate& iPos ) const;
    const float& GetNormed( const int& iRow, const int& iCol ) const;
    const float& GetNormed( const CartesianCoordinate& iPos ) const;

    CartesianCoordinate Center() const;
    
    float TemplateMatch(
        const Image&            iMask,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;

    float TemplateMatch(
        const Image&            iMask,
        const Rectangle&        iSearchWindow,
        CartesianCoordinate&    oBestMatch,
        Image*                  oCorrelationMap=NULL
    ) const;

    void SubImage(
        const Rectangle&    iRegion,
        Image&              oSubImage
    ) const;
    void SubImage(
        const int&  iX,
        const int&  iY,
        const int&  iWidth,
        const int&  iHeight,
        Image&      oSubImage
    ) const;
    Image FourierTransform() const;
    Image Difference( const Image& iOther ) const;
    float Correlation( const Image& iOther ) const;

private:
    void ResetMatrix();
    void RecalculateGreyLvl();
    void RecalculateNormalised();
};

#endif /* IMAGE_H_ */
