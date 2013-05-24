#ifndef _POINTSET_H_
#define _POINTSET_H_

#include <cstring>
#include <vector>
#include "Vertex.h"

class PointSet {
private:
    std::vector<Vertex>     m_vertices;
    
public:
    PointSet ( void );
    PointSet ( const std::string& iFilename );
    ~PointSet ( void );

    void LoadPlyFile ( const char* iFilename );
    void LoadFromFile ( const std::string& iFilename );
    void WritePlyFile ( const char* iFilename );
    void WriteToFile ( const std::string& iFilename );

    void PushVertex ( const Vertex& iVertex );
};

inline void PointSet::PushVertex (
    const Vertex& iVertex
) {
    m_vertices.push_back ( iVertex );
}

#endif
