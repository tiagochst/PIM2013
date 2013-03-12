#include <Color.h>


float fclamp(const float min, const float max, const float val) {
    return max(min, min(max, val));
}

Color::Color(int red, int green, int blue, int alpha) {
    m_R = (red) / (255.0f);
    m_G = (green) / (255.0f);
    m_B = (blue) / (255.0f);
    m_A = (alpha) / (255.0f);

    ClampValues();
}

Color::Color(const float red, const float green, const float blue, 
                const float alpha) {
    m_R = red;
    m_G = green;
    m_B = blue;
    m_A = alpha;

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
