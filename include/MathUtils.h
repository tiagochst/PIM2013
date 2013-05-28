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

#endif // _MATHUTILS_H_
