#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "Vec3D.hpp"
#include "Color.h"

class Vertex {
private:
    Vec3Df          m_position;
    Vec3Df          m_normal;
    Color           m_color;
    unsigned int    m_U;
    unsigned int    m_V;

public:
    // Constructors & destructors
    Vertex(void);
    Vertex(const Vec3Df& iPosition, const Vec3Df& iNormal, const Color& iColor);
    Vertex(const Vec3Df& iPosition, const Vec3Df& iNormal, const Color& iColor, const unsigned int& iU, const unsigned int& iV);
    Vertex(const Vertex& iSource);
    ~Vertex(void);

    // Accesssors
    const Vec3Df& GetPosition(void) const;
    void SetPosition(const Vec3Df& iPosition);

    const Vec3Df& GetNormal(void) const;
    void SetNormal(const Vec3Df& iNormal);

    const Color& GetColor(void) const;
    void SetColor(const Color& iColor);

    void GetUVCoord(unsigned int& oU, unsigned int& oV) const;
    void SetUVCoord(const unsigned int& iU, const unsigned int& iV);

    // Operators
    Vertex& operator=(const Vertex& iOther);
};

#endif
