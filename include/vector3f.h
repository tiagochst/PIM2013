class Vector3f {
private:
    float m_data[4];

public:
    Vector3f(void);
    Vector3f(const float& val);
    Vector3f(const float& x, const float& y, const float& z);
    ~Vector3f(void);
    
};

class Mesh3D {
private:
    int     m_data_stride;
    float*  m_
public:
}

class Color {
private:
    float   m_R;
    float   m_G;
    float   m_B;
    float   m_A;
    
public:
    Color(const int& RGB, const int& A=255);
    Color(const int& R, const int& G, const int& B, const int& A=255);
    
    Color(const float& RGB, const float& A=1.0f);
    Color(const float& R, const float& G, const float& B, const float& A=1.0f);
};

class Vertex {
private:
    Vector3f   m_position[4];  // Position in 3D space.
    Color      m_color[4];     // Color in RGBA coordinates.
    Vector3f   m_normal[3];    // Tangent plane normal.

public:
    Vertex(void);
    ~Vertex(void);
};