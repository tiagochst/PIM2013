#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <string>
#include "Eigen/Dense"
#include "ImageBase.h"

class Rectangle;
class CartesianCoordinate;

class Image
    :   public ImageBase
{
private:
    int                 m_height;
    int                 m_width;
    int                 m_maxGreyLevel;
    Eigen::MatrixXi*    m_figure;
    Eigen::MatrixXf*    m_normalisedFigure;

public:
    Image ();
    Image (
        const int& iWidth,
        const int& iHeight,
        const int& iGreyLevel
    );
    Image (
        const std::string& iFilename
    );
    virtual ~Image();

    /* Read a binary (P5) or Ascii(P2) .pgm file*/
    void LoadFromFile (
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

    // Interface Methods
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
        const int&  iGreyLevel,
        const bool& iRenormalise=false
    ); 
    int const& GetHeight () const;
    int const& GetWidth () const;
    int const& GetMaxGreyLevel () const;
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

private:
    void ClearMatrix ();
    void ResetMatrix ();
    void RecalculateGreyLvl ();
    void RecalculateNormalised ();
};

#endif /* IMAGE_H_ */
