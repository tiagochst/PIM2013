#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

class BadIndex {};
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
    int                     m_width;
    int                     m_height;

public:
    inline Rectangle( const int& iX, const int& iY, const int& iWidth, const int& iHeight )
        : m_position( iX, iY ), 
            m_width( iWidth ),
            m_height( iHeight ),
            m_center( iX + iWidth / 2, iY + iHeight / 2 )
    {}
    inline const int& GetWidth() const { return m_width; }
    inline const int& GetHeight() const { return m_height; } 
    inline const int& GetX() const { return m_position.x; }
    inline const int& GetY() const { return m_position.y; }
    inline const CartesianCoordinate& GetPosition() const { return m_position; }
    inline const CartesianCoordinate& GetCenter() const { return m_center; }
};

class Image {
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi     m_figure;
    Eigen::MatrixXf     m_normalisedFigure;

public:
    Image(const int iWidth, const int iHeight, const int iGreyLevel);
    Image(const std::string& iFilename);
    virtual ~Image();

    void SetHeight(const int iHeight);
    void SetWidth(const int iWidth) ;
    void SetGreyLevel(const int iGreyLevel); 

    int const& GetHeight() const;
    int const& GetWidth() const;
    int const& GetMaxGreyLevel() const;

    /* Read a binary (P5) or Asc(P2) .pgm file*/
    void LoadFromFile(const std::string& iFilename);

    /* Verify if image was read correctly*/
    void CreateAsciiPgm(const std::string& iFilename);

    void Recalculate();

    void SetGreyLvl( const int iRow, const int iCol, int iValue );
    void SetNormed( const int iRow, const int iCol, float iValue );
    void SetGreyLvl( const CartesianCoordinate& iPos, int iValue );
    void SetNormed( const CartesianCoordinate& iPos, float iValue );

    const int   GetGreyLvl( const int iRow, const int iCol ) const;
    const int   GetGreyLvl( const CartesianCoordinate& iPos ) const;
    const float GetNormed( const int iRow, const int iCol ) const;
    const float GetNormed( const CartesianCoordinate& iPos ) const;

    CartesianCoordinate Center() const;
    
    float TemplateMatch(
        const Image&            iMask,
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
};

#endif /* IMAGE_H_ */
