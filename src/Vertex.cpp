#include <Vertex.h>

// Constructors & destructors
Vertex::Vertex(void)
    : m_position(0, 0, 0), m_normal(0, 0, 0), m_color(1.0f, 1.0f, 1.0f, 1.0f)
{}

Vertex::Vertex(const Vec3Df& iPosition, const Vec3Df& iNormal, const Color& iColor)
    : m_position(iPosition), m_normal(iNormal), m_color(iColor)
{}

Vertex::Vertex(const Vertex& iSource)
{
    if ( this != &(iSource) ) {
        m_position = iSource.m_position;
        m_normal   = iSource.m_normal;
        m_color    = iSource.m_color;
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

// Operators
Vertex& Vertex::operator=(const Vertex& iOther)
{
    if ( this != &iOther ) {
        m_position  = iOther.m_position;
        m_normal    = iOther.m_normal;
        m_color     = iOther.m_color;
    }
    return (*this);
}
