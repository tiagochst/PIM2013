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

    const float& Red () const;
    const float& Green () const;
    const float& Blue () const;
    const float& Alpha () const;

    int Brightness () const;

private:
    void ClampValues();
};

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
