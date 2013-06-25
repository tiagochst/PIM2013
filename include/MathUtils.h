#ifndef _MATHUTILS_H_
#define _MATHUTILS_H_

#include <Vec3D.hpp>
#include <limits>
#include <cmath>

inline bool my_isnan (
    const float&    iValue
) {
    const unsigned char* cval = reinterpret_cast<const unsigned char*>(&iValue);

    static const float nqnan = -std::numeric_limits<float>::quiet_NaN();
    bool nqnantest = true;
    const unsigned char* cnan = reinterpret_cast<const unsigned char*>(&nqnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        nqnantest &= (cval[i] == cnan[i]);
    }
    static const float qnan = std::numeric_limits<float>::quiet_NaN();
    bool qnantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&qnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        qnantest &= (cval[i] == cnan[i]);
    }

    static const float nsnan = -std::numeric_limits<float>::signaling_NaN();
    bool nsnantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&nsnan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        nsnantest &= (cval[i] == cnan[i]);
    }
    static const float snan = std::numeric_limits<float>::signaling_NaN();
    bool snantest = true;
    cnan = reinterpret_cast<const unsigned char*>(&snan);
    for ( unsigned int i = 0; i < sizeof (float); i++ ) {
        snantest &= (cval[i] == cnan[i]);
    }

    return (snantest || qnantest) || (nsnantest || nqnantest);
} 

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


struct CartCoordf {
    float x;
    float y;

    inline CartCoordf ()
        : x ( 0 ), y ( 0 )
    {}
    inline CartCoordf (
        const float& x,
        const float& y
    )   : x ( x ), y ( y )
    {}
};
inline bool operator<(const CartCoordf& iLhs, const CartCoordf& iRhs)
{
    return iLhs.x<iRhs.x || (!(iRhs.x<iLhs.x) && iLhs.y<iRhs.y);
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

class Matrix33 {
private:
    Vec3Df  m_col0;
    Vec3Df  m_col1;
    Vec3Df  m_col2;

public:
    inline Matrix33 ( const Vec3Df& iCol0, const Vec3Df& iCol1, const Vec3Df& iCol2 )
        :   m_col0 ( iCol0 ), m_col1 ( iCol1 ), m_col2 ( iCol2 )
    {}
    inline ~Matrix33 () {}

    inline float Det () const {
        float det = 0;

        det += m_col0[0] * (m_col1[1]*m_col2[2] - m_col2[1]*m_col1[2]);
        det -= m_col1[0] * (m_col0[1]*m_col2[2] - m_col2[1]*m_col0[2]);
        det += m_col2[0] * (m_col0[1]*m_col1[2] - m_col1[1]*m_col0[2]);

        return det;
    }
};

class Parabola {
private:
    float   m_A;
    float   m_B;
    float   m_C;

public:
    inline Parabola (
        const float& x_1, const float& y_1,
        const float& x0, const float& y0,
        const float& x1, const float& y1
    ) {
        Matrix33 M (
            Vec3Df ( x_1*x_1, x0*x0, x1*x1),
            Vec3Df ( x_1, x0, x1),
            Vec3Df ( 1.0f, 1.0f, 1.0f )
        );
        Matrix33 Ma (
            Vec3Df ( y_1, y0, y1),
            Vec3Df ( x_1, x0, x1),
            Vec3Df ( 1.0f, 1.0f, 1.0f )
        );
        Matrix33 Mb (
            Vec3Df ( x_1*x_1, x0*x0, x1*x1),
            Vec3Df ( y_1, y0, y1),
            Vec3Df ( 1.0f, 1.0f, 1.0f )
        );
        Matrix33 Mc (
            Vec3Df ( x_1*x_1, x0*x0, x1*x1),
            Vec3Df ( x_1, x0, x1 ),
            Vec3Df ( y_1, y0, y1 )
        );
        const float Mdet = M.Det ();

        m_A = Ma.Det () / Mdet;
        m_B = Mb.Det () / Mdet;
        m_C = Mc.Det () / Mdet;
    }
    inline Parabola (
        const float& iA, const float& iB, const float& iC
    ) {
        m_A = iA; m_B = iB; m_C = iC;
    }
    inline float Eval ( const float& iX ) const {
        return m_A * (iX*iX) + m_B * iX + m_C;
    }
    inline float Max ( const float& iMinX, const float& iMaxX, float& oMaxX ) const {
        float yMin = Eval ( iMinX );
        float yMax = Eval ( iMaxX );

        float xExt = -m_B / (2 * m_A);
        float yExt = Eval ( xExt );

        if ( yMax > yExt && yMax > yMin ) {
            oMaxX = iMaxX;
            return yMax; 
        } else if ( yMin > yExt && yMin > yMax ) {
            oMaxX = iMinX;
            return yMin;
        } else {
            oMaxX = xExt;
            return yExt;
        }
    }
    inline float Min ( const float& iMinX, const float& iMaxX, float& oMinX ) const {
        float yMin = Eval ( iMinX );
        float yMax = Eval ( iMaxX );

        float xExt = -m_B / (2 * m_A);
        float yExt = Eval ( xExt );

        if ( yMax < yExt && yMax < yMin ) {
            oMinX = iMaxX;
            return yMax; 
        } else if ( yMin < yExt && yMin < yMax ) {
            oMinX = iMinX;
            return yMin;
        } else {
            oMinX = xExt;
            return yExt;
        }
    }
};

inline Vec3Df BilinearInterpolation (
    const Vec3Df&   f00,
    const Vec3Df&   f01,
    const Vec3Df&   f10,
    const Vec3Df&   f11,
    const float&    px0,
    const float&    py0,
    const float&    px1,
    const float&    py1,
    const float&    px,
    const float&    py
) {
    float denomX = (px1-px0);
    float denomY = (py1-py0);
    float denom = denomX*denomY;

    if ( denomX != 0.0f && denomY != 0.0f ) {
        Vec3Df fx0 = ((px1-px)*f00+(px-px0)*f10)/denomX;
        Vec3Df fx1 = ((px1-px)*f01+(px-px0)*f11)/denomX;

        return ((py1-py)*fx0 + (py-py0)*fx1)/denom;
    } else if ( denomY != 0.0f ) {
        return ((py1-py)*f00+(py-py0)*f01)/denomY;
    } else if ( denomX != 0.0f ) {
        return ((px1-px)*f00+(px-px0)*f10)/denomX;
    } else {
        return f00;
    }
}

#endif // _MATHUTILS_H_
