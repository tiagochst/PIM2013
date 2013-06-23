#include <Vertex.h>

// Constructors & destructors
Vertex::Vertex(void)
    : m_position(0, 0, 0), m_normal(0, 0, 0), m_color(1.0f, 1.0f, 1.0f, 1.0f), m_U (0u), m_V (0u)
{}

Vertex::Vertex(const Vec3Df& iPosition, const Vec3Df& iNormal, const Color& iColor)
    : m_position(iPosition), m_normal(iNormal), m_color(iColor), m_U (0.0f), m_V (0.0f)
{}

Vertex::Vertex(const Vec3Df& iPosition, const Vec3Df& iNormal, const Color& iColor, const float& iU, const float& iV)
    : m_position(iPosition), m_normal(iNormal), m_color(iColor), m_U(iU), m_V(iV)
{}

Vertex::Vertex(const Vertex& iSource)
{
    if ( this != &(iSource) ) {
        m_position  = iSource.m_position;
        m_normal    = iSource.m_normal;
        m_color     = iSource.m_color;
        m_U         = iSource.m_U;
        m_V         = iSource.m_V;
    }
}
Vertex::~Vertex(void)
{}

// Accesssors
const Vec3Df& Vertex::GetPosition(void) const
{
    return (m_position);
}

void Vertex::SetPosition(const Vec3Df& iPosition)
{
    m_position = iPosition;
}

const Vec3Df& Vertex::GetNormal(void) const
{
    return (m_normal);
}

void Vertex::SetNormal(const Vec3Df& iNormal)
{
    m_normal = iNormal;
}

const Color& Vertex::GetColor(void) const
{
    return (m_color);
}

void Vertex::SetColor(const Color& iColor)
{
    m_color = iColor;
}

void Vertex::GetUVCoord(float& oU, float& oV) const
{
    oU = m_U;
    oV = m_V;
}

void Vertex::SetUVCoord(const float& iU, const float& iV)
{
    m_U = iU;
    m_V = iV;
}

// Operators
Vertex& Vertex::operator=(const Vertex& iOther)
{
    if ( this != &iOther ) {
        m_position  = iOther.m_position;
        m_normal    = iOther.m_normal;
        m_color     = iOther.m_color;
        m_U         = iOther.m_U;
        m_V         = iOther.m_V;
    }
    return (*this);
}
