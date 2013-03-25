#ifndef _COLOR_H_
#define _COLOR_H_

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

private:
    void ClampValues();
};

#endif
