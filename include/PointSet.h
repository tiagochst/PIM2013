#ifndef _POINTSET_H_
#define _POINTSET_H_

#include <cstring>
#include <vector>
#include "Vertex.h"

struct Face {
    unsigned int v0, v1, v2;
};

class PointSet {
private:
    unsigned int            m_vtxCount;
    unsigned int            m_faceCount;
    std::vector<Vertex>     m_vertices;
    std::vector<Face>       m_faces;
    
public:
    PointSet ( void );
    PointSet ( const std::string& iFilename );
    ~PointSet ( void );

    void LoadPlyFile ( const char* iFilename );
    void LoadFromFile ( const std::string& iFilename );
    void WritePlyFile ( const char* iFilename );
    void WriteToFile ( const std::string& iFilename );

    void PushVertex ( const Vertex& iVertex );
    void PushFace ( const Face& iFace );

    void MoveToBarycenter ();

    void Draw () const;

    const Vertex& GetVertex ( const unsigned int& iVertex ) const;
    const Face& GetFace ( const unsigned int& iFace ) const;

    const unsigned int& GetNumVertices () const;
    const unsigned int& GetNumFaces () const;
};

inline void PointSet::PushVertex (
    const Vertex& iVertex
) {
    m_vtxCount++;
    m_vertices.push_back ( iVertex );
}

inline void PointSet::PushFace (
    const Face& iFace
) {
    m_faceCount++;
    m_faces.push_back ( iFace );
}

inline const Vertex& PointSet::GetVertex ( const unsigned int& iVertex ) const {
    return m_vertices[iVertex];
}
inline const Face& PointSet::GetFace ( const unsigned int& iFace ) const {
    return m_faces[iFace];
}

inline const unsigned int& PointSet::GetNumVertices ()
const {
    return m_vtxCount;
}

inline const unsigned int& PointSet::GetNumFaces ()
const {
    return m_faceCount;
}
 
#endif
