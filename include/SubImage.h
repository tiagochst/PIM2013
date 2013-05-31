#ifndef _SUBIMAGE_H_
#define _SUBIMAGE_H_

#include "Rectangle.h"
#include "ImageBase.h"

class Image;
struct CartesianCoordinate;

class SubImage
    :   public ImageBase
{
private:
    ImageBase*          m_parent;
    Rectangle           m_region;
    
public:
    SubImage (); 
    SubImage (
        ImageBase*  iParent
    );
    SubImage (
        ImageBase*          iParent,
        const Rectangle&    iRegion
    );

    void SetOffset (
        const int&  iOffsetX,
        const int&  iOffsetY
    );

    const int& GetParentWidth () const;
    const int& GetParentHeight () const;

    void SetParent (
        ImageBase*  iParent
    );

    // Interface Implementations
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

};

#endif // _SUBIMAGE_H_
