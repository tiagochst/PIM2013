#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

class BadIndex {};
class IncompatibleImages {};

struct CartesianCoordinate {
    int x, y;
    CartesianCoordinate() {
        this->x = this->y = 0;
    }
    CartesianCoordinate( const int& x, const int& y ) {
        this->x = x;
        this->y = y;
    }
};
struct Rectangle {
    CartesianCoordinate m_position;
    int width, height;
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
    
    Image SubImage( const int iX, const int iY, const int iWidth, const int iHeight ) const;
    Image TemplateMatch( const Image& iMask, CartesianCoordinate& oBestMatch ) const;
    Image FourierTransform() const;
    Image Difference( const Image& iOther ) const;
    float Correlation( const Image& iOther ) const;

private:
    void ResetMatrix();
};

#endif /* IMAGE_H_ */

