#include "PointSet.h"
#include "PlyFile.h"
#include <string>

typedef struct VertexPOD {
    float           x, y, z; // Position
    float           nx, ny, nz; // Normal
    unsigned char   red, green, blue, alpha; // Color
} VertxPOD;

typedef struct FacePOD {
    unsigned char   nVerts;    /* number of vertex indices in list */
    int*            verts;     /* vertex index list */
} FacePOD;

const static PlyProperty vertexProperties[] = {
    {             "x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,     x), 0,         0,         0,                      0},
    {             "y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,     y), 0,         0,         0,                      0},
    {             "z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,     z), 0,         0,         0,                      0},
    {            "nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,    nx), 0,         0,         0,                      0},
    {            "ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,    ny), 0,         0,         0,                      0},
    {            "nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,    nz), 0,         0,         0,                      0},
    {           "red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,   red), 0,         0,         0,                      0},
    {         "green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, green), 0,         0,         0,                      0},
    {          "blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,  blue), 0,         0,         0,                      0},
    {         "alpha", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex, alpha), 0,         0,         0,                      0},
};
const static PlyProperty faceProperties[] = {
    {"vertex_indices",   PLY_INT,   PLY_INT, offsetof(  Face, verts), 1, PLY_UCHAR, PLY_UCHAR, offsetof(Face, nVerts)},
};

PointSet::PointSet(void) {}
PointSet::PointSet(const std::string& iFilename) {
    LoadFromFile(filename);
}
PointSet::~PointSet(void) {}

void PointSet::LoadPlyFile(const char* iFilename) {
    PlyFile*        plyFile;
    int             nGlobalElems;
    int             fileType;
    float           version;
    
    int             nComments;
    char**          comments;
    int             nElems;
    char*           elementName;
    char**          elementNameList;
    int             nObjectInfo;
    char**          objectInfo;
    int             nProperties;
    PlyProperty**   propertyList;

    /* open a PLY file for reading */
    plyFile = ply_open_for_reading(iFilename, &nGlobalElems, &elementNameList, &fileType, &version);

    /* print what we found out about the file */
    std::cout << "version " << version  << std::endl;
    std::cout << "type    " << fileType << std::endl;

    /* go through each kind of element that we learned is in the file */
    /* and read them */
    for ( int elem = 0; elem < nGlobalElems; elem++ ) {
        /* get the description of the first element */
        std::string elementName(elementNameList[elem]);
        plist = ply_get_element_description (plyFile, elementName, &nElems, &nProperties);

        /* print the name of the element, for debugging */
#ifdef __DEBUG_PLY_READ
        std::cout << "element " << elementName << " " << nElems << std::endl;
#endif

        /* if we're on vertex elements, read them in */
        if ( elementName.compare("vertex") ) {
            /* set up for getting vertex elements */
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[0]); // x
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[1]); // y
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[2]); // z
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[3]); // nx
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[4]); // ny
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[5]); // nz
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[6]); // red
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[7]); // green
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[8]); // blue
            ply_get_property(plyFile, elementName.c_str(), &vertexProperties[9]); // alpha
                                                 
            /* grab all the vertex elements */   
            for ( int vertex = 0; vertex < nElems; vertex++ ) {
                /* grab and element from the file */
                VertexPOD vDesc;
                ply_get_element(plyFile, (void*)&v);

                Vec3Df p(vDesc.x , vDesc.y , vDesc.z );
                Vec3Df n(vDesc.nx, vDesc.ny, vDesc.nz);
                Color  c(vDesc.red, vDesc.green, vDesc.blue, vDesc.alpha);
                Vertex v(p, n, c);
                
                m_vertices.push_back(v);
                
#ifdef __DEBUG_PLY_READ
                /* print out vertex x,y,z for debugging */
                std::cout << "Read Vertex:" << std::endl;
                std::cout << " - Position = " << vDesc.x   << " " << vDesc.y     << " " << vDesc.z    << std::endl;
                std::cout << " - Normal   = " << vDesc.nx  << " " << vDesc.ny    << " " << vDesc.nz   << std::endl;
                std::cout << " - Color    = " << vDesc.red << " " << vDesc.green << " " << vDesc.blue << " " << vDesc.alpha << std::endl;
#endif
            }
        }

        /* if we're on face elements, read them in */
        if ( elementName.compare("face") == 0 ) {
            /* set up for getting face elements */
            ply_get_property(plyFile, elementName.c_str(), &faceProperties[0]);

            /* grab all the face elements */
            for ( int face = 0; face < nElems; face++ ) {
                /* grab and element from the file */
                FacePOD f;
                ply_get_element(plyFile, (void*)&(FacePOD));

#ifdef __DEBUG_PLY_READ
                /* print out face info, for debugging */
                std::cout << f.nVerts << " ";
                for ( int vert = 0; vert < f.nVerts; vert++ ) {
                    std::cout << verts[vert]) << " ";
                }
                std::cout << std::endl;
#endif
            }
        }

#ifdef __DEBUG_PLY_READ
        /* print out the properties we got, for debugging */
        for ( int prop = 0; prop < nProperties; prop++ ) {
            std::cout << "property " << propertyList[prop]->name << std::endl;
        }
#endif
    }

    /* grab and print out the comments in the file */
    comments = ply_get_comments(plyFile, &nComments);
    for ( int comm = 0; comm < nComments; comm++ ) {
        std::cout << "comment = " << comments[comm] << std::endl; 
    }

    /* grab and print out the object information */
    obj_info = ply_get_obj_info(plyFile, &nObjInfo);
    for ( int objInfo = 0; i < nObjInfo; objInfo++) {
        std::cout << "obj_info = " << objectInfo[objInfo] << std::endl;
    }

    /* close the PLY file */
    ply_close(ply);
}

void PointSet::LoadFromFile(const std::string& iFilename) {
    int plyPos = iFilename.find(".ply");
    if ( (plyPos != std::string::npos) && 
            (plyPos != (iFilename.size() - 4)) ) {
        LoadPlyFile(iFilename.c_str());
    }
}
void PointSet::WriteToFile(const std::string& iFilename) {}