#ifndef _COLOR_H_
#define _COLOR_H_

#include <cmath>

class Color {
private:
    float    m_R; 
    float    m_G; 
    float    m_B; 
    float    m_A; 

public:

    Color();
    Color(const Color& iOther);
    Color(int red, int green, int blue, int alpha=255);
    Color(const float red, const float green, const float blue, 
            const float alpha=1.0f);
    ~Color(void);
    Color& operator=(const Color& iOther);

    const float& Red () const;
    const float& Green () const;
    const float& Blue () const;
    const float& Alpha () const;

    int Brightness () const;

    void FromHSV (
        const float&    iHue,
        const float&    iSaturation,
        const float&    iValue
    );

private:
    void ClampValues();
};

inline void Color::FromHSV (
    const float&    iHue,
    const float&    iSaturation,
    const float&    iValue
) {
    unsigned int hi = ((unsigned int)floor ( iHue / 60u ) % 6);
    float f = (iHue / 60.0f) - hi;

    float l = iValue * ( 1.0f - iSaturation );
    float m = iValue * ( 1.0f - ( f * iSaturation ) );
    float n = iValue * ( 1.0f - ( 1.0f - f ) * iSaturation );

    m_A = 1.0f;
    switch ( hi ) {
        case 0:
            m_R = iValue;
            m_G = n;
            m_B = l;
            break;

        case 1:
            m_R = m;
            m_G = iValue;
            m_B = l;
            break;

        case 2:
            m_R = l;
            m_G = iValue;
            m_B = n;
            break;

        case 3:
            m_R = l;
            m_G = m;
            m_B = iValue;
            break;

        case 4:
            m_R = n;
            m_G = l;
            m_B = iValue;
            break;

        case 5:
            m_R = iValue;
            m_G = l;
            m_B = m;
            break; 
    }
}

inline int Color::Brightness ()
const {
    return (int)(
        255 * (
                m_R * 0.2126f
            +   m_G * 0.7152f
            +   m_B * 0.0722f
        )
    );
}

inline const float& Color::Red () 
const {
    return m_R;
}

inline const float& Color::Green () 
const {
    return m_G;
}

inline const float& Color::Blue () 
const {
    return m_B;
}

inline const float& Color::Alpha () 
const {
    return m_A;
}

#endif
