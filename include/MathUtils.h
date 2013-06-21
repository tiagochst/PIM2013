#ifndef _MATHUTILS_H_
#define _MATHUTILS_H_

#include <Vec3D.hpp>

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

    inline bool operator<(const CartesianCoordinate& iOther) const
    {
        return (x<iOther.x)&&(y<iOther.y);
    }
    inline bool operator>(const CartesianCoordinate& iOther) const
    {
        return (x>iOther.x)&&(y>iOther.y);
    }
    inline bool operator<=(const CartesianCoordinate& iOther) const
    {
        return (x<=iOther.x)&&(y<=iOther.y);
    }
    inline bool operator>=(const CartesianCoordinate& iOther) const
    {
        return (x>=iOther.x)&&(y>=iOther.y);
    }
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

#endif // _MATHUTILS_H_
