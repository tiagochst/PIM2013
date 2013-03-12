#include <Color.h>
#define max(x, y) ((x > y) ? x : y)
#define min(x, y) ((x < y) ? x : y)

float fclamp(const float iMin, const float iMax, const float iVal) {
    return max(iMin, min(iMax, iVal));
}

Color::Color(const Color& iSource) {
    if ( this != &(iOther) ) {
        m_R = iOther.m_R; 
        m_G = iOther.m_G; 
        m_B = iOther.m_B; 
        m_A = iOther.m_A;
    }
}
Color& operator=(const Color& iOther) {
    if ( this != &(iOther) ) {
        m_R = iOther.m_R; 
        m_G = iOther.m_G; 
        m_B = iOther.m_B; 
        m_A = iOther.m_A;
    }
}

Color::Color(int iRed, int iGreen, int iBlue, int iAlpha) {
    m_R = (iRed) / (255.0f);
    m_G = (iGreen) / (255.0f);
    m_B = (iBlue) / (255.0f);
    m_A = (iAlpha) / (255.0f);

    ClampValues();
}

Color::Color(const float iRed, const float iGreen, const float iBlue, 
                const float iAlpha) {
    m_R = iRed;
    m_G = iGreen;
    m_B = iBlue;
    m_A = iAlpha;

    ClampValues();
}

Color::~Color(void) {
}

void Color::ClampValues(void) {
    m_R = fclamp(0.0f, 1.0f, m_R);
    m_G = fclamp(0.0f, 1.0f, m_G);
    m_B = fclamp(0.0f, 1.0f, m_B);
    m_A = fclamp(0.0f, 1.0f, m_A);
}
