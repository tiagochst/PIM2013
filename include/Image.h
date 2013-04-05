#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

class BadIndex {};
class IncompatibleImages {};

struct Coordinate {
    int x, y;
    Coordinate() {
        this->x = this->y = 0;
    }
    Coordinate(const int& x, const int& y) {
        this->x = x;
        this->y = y;
    }
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

    float& operator()( const int iRow, const int iCol );
    float  operator()( const int iRow, const int iCol ) const;

    Image PatternSearch( const Image& iMask, Coordinate& oBestMatch ) const;
    int Correlation( const Image& iOther ) const;
    Image Difference( const Image& iOther) const;
};

#endif /* IMAGE_H_ */


