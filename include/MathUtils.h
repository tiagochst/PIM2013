#ifndef _MATHUTILS_H_
#define _MATHUTILS_H_

inline bool IsOdd (
    const int iVal
) {
    return !( iVal & 0x1 );
}

template<typename T>
inline bool InRange( 
    const T& iVal, 
    const T& iMin, 
    const T& iMax
) {
    return ( ( iVal >= iMin  ) && ( iVal <= iMax ) );
}

//inline int abs (
//    const int& iVal
//) {
//    return ( iVal < 0 ) ? -iVal : iVal;
//}

template<typename T>
inline T min (
    const T& a,
    const T& b
) {
    return ( ( a <= b ) ? a : b );    
}
template<typename T>
inline T max (
    const T& a,
    const T& b
) {
    return ( ( a >= b ) ? a : b );    
}

struct CartesianCoordinate {
    int x;
    int y;

    inline CartesianCoordinate ()
        : x ( 0 ), y ( 0 )
    {}
    inline CartesianCoordinate (
        const int& x,
        const int& y
    )   : x ( x ), y ( y )
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

#endif // _MATHUTILS_H_
