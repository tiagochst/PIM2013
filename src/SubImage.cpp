#include <cassert>
#include "MathUtils.h"
#include "ImageBase.h"
#include "Image.h"
#include "SubImage.h"

SubImage::SubImage ()
    :   ImageBase (),
        m_parent ( 0x0 ),
        m_region ()
{}
SubImage::SubImage (
    ImageBase*  iParent
)   :   ImageBase (),
        m_parent ( iParent ),
        m_region ()
{
    assert ( m_parent != (ImageBase*)0x0 );
}
SubImage::SubImage (
    ImageBase*          iParent,
    const Rectangle&    iRegion
)   :   ImageBase (),
        m_parent ( iParent ),
        m_region ( iRegion )
{
    assert ( m_parent != (ImageBase*)0x0 );
}

void SubImage::SetOffset (
    const int&  iOffsetX,
    const int&  iOffsetY
) {
    m_region.SetX ( iOffsetX );
    m_region.SetY ( iOffsetY );
}

const int& SubImage::GetParentWidth ()
const {
    assert ( m_parent != (ImageBase*)0x0 );
    return m_parent->GetWidth ();
}
const int& SubImage::GetParentHeight ()
const {
    assert ( m_parent != (ImageBase*)0x0 );
    return m_parent->GetHeight ();
}

void SubImage::SetParent (
    ImageBase*  iParent
) {
    assert ( iParent != (ImageBase*)0x0 );
    m_parent = iParent;
}

void SubImage::SetDimensions (
    const int& iWidth,
    const int& iHeight
) {
    m_region.SetWidth ( iWidth );
    m_region.SetHeight ( iHeight );
}
void SubImage::SetHeight (
    const int& iHeight
) {
    m_region.SetHeight ( iHeight );
}
void SubImage::SetWidth (
    const int& iWidth
) {
    m_region.SetWidth ( iWidth );
}
void SubImage::SetMaxGreyLevel (
    const int&  iGreyLevel,
    const bool& iRenormalise
) {
    m_parent->SetMaxGreyLevel (
        iGreyLevel,
        iRenormalise
    );
} 

int const& SubImage::GetHeight ()
const {
    return m_region.Height ();
}
int const& SubImage::GetWidth ()
const {
    return m_region.Width ();
}
int const& SubImage::GetMaxGreyLevel ()
const {
    assert ( m_parent != (ImageBase*)0x0 );
    return m_parent->GetMaxGreyLevel ();
}

inline const int& SubImage::GetGreyLvl (
    const CartesianCoordinate& iPos
) const {
    return GetGreyLvl ( iPos.y, iPos.x );
}
const int& SubImage::GetGreyLvl (
    const int& iRow,
    const int& iCol
) const {
    assert ( m_parent != (ImageBase*)0x0 );

    int row = abs ( iRow + m_region.Y() );
    int col = abs ( iCol + m_region.X() );

    row = IsOdd (row / GetHeight ()) ? (row % GetHeight ()) : (GetHeight () - (row % GetHeight ()) - 1);
    col = IsOdd (col / GetWidth () ) ? (col % GetWidth () ) : (GetWidth ()  - (col % GetWidth () ) - 1);
    
    return m_parent->GetGreyLvl ( row, col );
}
inline const float& SubImage::GetNormed (
    const CartesianCoordinate& iPos
) const {
    return GetNormed ( iPos.y, iPos.x );
}
inline const float& SubImage::GetNormed (
    const int& iRow,
    const int& iCol
) const {
    assert ( m_parent != (ImageBase*)0x0 );

    int row = abs ( iRow + m_region.Y() );
    int col = abs ( iCol + m_region.X() );

    row = IsOdd (row / GetHeight ()) ? (row % GetHeight ()) : (GetHeight () - (row % GetHeight ()) - 1);
    col = IsOdd (col / GetWidth () ) ? (col % GetWidth () ) : (GetWidth ()  - (col % GetWidth () ) - 1);
    
    return m_parent->GetNormed ( row, col );
}

inline void SubImage::SetGreyLvl (
    const int& iRow,
    const int& iCol,
    const int& iValue
) {
    assert ( m_parent != (ImageBase*)0x0 );
    m_parent->SetGreyLvl ( iRow, iCol, iValue );
}

inline void SubImage::SetNormed (
    const int&      iRow,
    const int&      iCol,
    const float&    iValue
) {
    assert ( m_parent != (ImageBase*)0x0 );
    m_parent->SetNormed ( iRow, iCol, iValue );
}

inline void SubImage::SetGreyLvl (
    const CartesianCoordinate&  iPos,
    const int&                  iValue
) {
    SetGreyLvl ( iPos.y, iPos.x, iValue );
}

inline void SubImage::SetNormed (
    const CartesianCoordinate&  iPos,
    const float&                iValue
) {
    SetNormed ( iPos.y, iPos.x, iValue );
}

