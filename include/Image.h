#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"

class BadIndex {};

class Image {
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi     m_figure;

public:
    Image(const int iWidth, const int iHeight, const int iGreyLevel);
    Image(const std::string& iFilename);
    virtual ~Image();

    void SetHeight(const int iHeight);
    void SetWidth(const int iWidth) ;
    void SetGreyLevel(const int iGreyLevel); 

    int GetHeight();
    int GetWidth();
    int GetMaxGreyLevel();

    /* Read a binary (P5) or Asc(P2) .pgm file*/
    void LoadFromFile(const std::string& iFilename);

    /* Verify if image was read correctly*/
    void CreateAsciiPgm(const std::string& iFilename);

    int& operator()( const int iRow, const int iCol );
    int  operator()( const int iRow, const int iCol ) const;

    Image Correlation( const Image& iOther ) const;
    Image Difference( const Image& iOther) const;
};

#endif /* IMAGE_H_ */


