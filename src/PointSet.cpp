#include "PointSet.h"
#include "PlyFile.h"
#include <string>
#include <GL/gl.h>
#include <GL/glu.h>

typedef struct VertexPOD {
    float           x, y, z; // Position
    float           nx, ny, nz; // Normal
    unsigned int    u, v; //UV coordinates
    unsigned char   red, green, blue, alpha; // Color
} VertxPOD;

typedef struct FacePOD {
    unsigned char   nVerts;    /* number of vertex indices in list */
    unsigned int*   verts;     /* vertex index list */
} FacePOD;

static PlyProperty vertexProperties[] = {
    {             "x", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,     x), 0,         0,         0,                         0},
    {             "y", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,     y), 0,         0,         0,                         0},
    {             "z", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,     z), 0,         0,         0,                         0},
    {            "nx", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,    nx), 0,         0,         0,                         0},
    {            "ny", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,    ny), 0,         0,         0,                         0},
    {            "nz", PLY_FLOAT, PLY_FLOAT, offsetof(VertexPOD,    nz), 0,         0,         0,                         0},
    {             "u",  PLY_UINT,  PLY_UINT, offsetof(VertexPOD,     u), 0,         0,         0,                         0},
    {             "v",  PLY_UINT,  PLY_UINT, offsetof(VertexPOD,     v), 0,         0,         0,                         0},
    {           "red", PLY_UCHAR, PLY_UCHAR, offsetof(VertexPOD,   red), 0,         0,         0,                         0},
    {         "green", PLY_UCHAR, PLY_UCHAR, offsetof(VertexPOD, green), 0,         0,         0,                         0},
    {          "blue", PLY_UCHAR, PLY_UCHAR, offsetof(VertexPOD,  blue), 0,         0,         0,                         0},
    {         "alpha", PLY_UCHAR, PLY_UCHAR, offsetof(VertexPOD, alpha), 0,         0,         0,                         0},
};
static PlyProperty faceProperties[] = {
    {"vertex_indices",  PLY_UINT,  PLY_UINT, offsetof(  FacePOD, verts), 1, PLY_UCHAR, PLY_UCHAR, offsetof(FacePOD, nVerts)},
};

PointSet::PointSet(void)
    :   m_vtxCount ( 0u ), m_faceCount ( 0u )
{}
PointSet::PointSet(const std::string& iFilename)
    :   m_vtxCount ( 0u ), m_faceCount ( 0u )
{
    LoadFromFile(iFilename);
}
PointSet::~PointSet(void) {}

void PointSet::WritePlyFile(const char* iFilename) {
    int i,j;
    PlyFile *plyFile;
    int file_type;
    float version;

    /* open either an ascii PLY file for writing */
    /* (the file will be called "test.ply" because the routines */
    /*  enforce the .ply filename extension) */
    size_t          pathLen = strlen(iFilename);
    char*           filename = new char[pathLen + 1];
    memcpy(filename, iFilename, (pathLen + 1) * sizeof(char));

    char* elem_names[] = { "vertex", "face" };

    /* open a PLY file for reading */
    plyFile = ply_open_for_writing(filename, 2, elem_names, PLY_ASCII, &version);
    delete[] filename;
    filename = 0;

    /* describe what properties go into the vertex and face elements */
    ply_element_count ( plyFile, "vertex", m_vertices.size () );

    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 0] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 1] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 2] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 3] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 4] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 5] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 6] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 7] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 8] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[ 9] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[10] );
    ply_describe_property ( plyFile, "vertex", &vertexProperties[11] );

    ply_element_count ( plyFile, "face", m_faces.size () );

    ply_describe_property ( plyFile, "face", &faceProperties[0] );

    ply_put_comment (plyFile, "author: PIM2013");

    /* we have described exactly what we will put in the file, so */
    /* we are now done with the header info */
    ply_header_complete ( plyFile );

    /* set up and write the vertex elements */
    ply_put_element_setup ( plyFile, "vertex" );
    for (i = 0; i < m_vertices.size (); i++) {
        Vertex vtx = m_vertices[i];

        const Vec3Df&   position    =   vtx.GetPosition ();
        const Vec3Df&   normal      =   vtx.GetNormal ();
        const Color&    color       =   vtx.GetColor ();
        unsigned int    u           =   0;
        unsigned int    v           =   0;

        vtx.GetUVCoord ( u, v );

        VertexPOD vPOD;
        vPOD.x      = position[0];
        vPOD.y      = position[1];
        vPOD.z      = position[2];
        vPOD.nx     = normal[0];
        vPOD.ny     = normal[1];
        vPOD.nz     = normal[2];
        vPOD.u      = u; 
        vPOD.v      = v; 
        vPOD.red    = 255.f * color.Red ();
        vPOD.green  = 255.f * color.Green ();
        vPOD.blue   = 255.f * color.Blue ();
        vPOD.alpha  = 255.f * color.Alpha ();

        ply_put_element ( plyFile, (void *) &vPOD );
    }

    ply_put_element_setup ( plyFile, "face" );
    for (i = 0; i < m_faces.size (); i++) {
        Face face = m_faces[i];

        FacePOD fPOD;
        fPOD.nVerts = 3;
        fPOD.verts = new unsigned int[3];
        fPOD.verts[0] = face.v0;
        fPOD.verts[1] = face.v1;
        fPOD.verts[2] = face.v2;

        ply_put_element ( plyFile, (void *) &fPOD );

        delete[] fPOD.verts;
        fPOD.verts = (unsigned int*)0x0;
    }

    /* close the PLY file */
    ply_close ( plyFile );
}

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

    size_t          pathLen = strlen(iFilename);
    char*           filename = new char[pathLen + 1];
    memcpy(filename, iFilename, (pathLen + 1) * sizeof(char));

    /* open a PLY file for reading */
    plyFile = ply_open_for_reading(filename, &nGlobalElems, &elementNameList, &fileType, &version);
    delete[] filename;
    filename = 0;

    /* print what we found out about the file */
    std::cout << "version " << version  << std::endl;
    std::cout << "type    " << fileType << std::endl;

    /* go through each kind of element that we learned is in the file */
    /* and read them */
    for ( int elem = 0; elem < nGlobalElems; elem++ ) {
        /* get the description of the first element */
        std::string elementName(elementNameList[elem]);
        char* cElementName = elementNameList[elem];
        propertyList = ply_get_element_description(plyFile, cElementName, &nElems, &nProperties);

        /* print the name of the element, for debugging */
#ifdef __DEBUG_PLY_READ
        std::cout << "element " << elementName << " " << nElems << std::endl;

        /* print out the properties we got, for debugging */
        for ( int prop = 0; prop < nProperties; prop++ ) {
            std::cout << "property " << propertyList[prop]->name << std::endl;
        }
#endif

        /* if we're on vertex elements, read them in */
        if ( elementName.compare("vertex") == 0 ) {
            /* set up for getting vertex elements */
            ply_get_property(plyFile, cElementName, &vertexProperties[ 0]); // x
            ply_get_property(plyFile, cElementName, &vertexProperties[ 1]); // y
            ply_get_property(plyFile, cElementName, &vertexProperties[ 2]); // z
            ply_get_property(plyFile, cElementName, &vertexProperties[ 3]); // nx
            ply_get_property(plyFile, cElementName, &vertexProperties[ 4]); // ny
            ply_get_property(plyFile, cElementName, &vertexProperties[ 5]); // nz
            ply_get_property(plyFile, cElementName, &vertexProperties[ 6]); // u
            ply_get_property(plyFile, cElementName, &vertexProperties[ 7]); // v
            ply_get_property(plyFile, cElementName, &vertexProperties[ 8]); // red
            ply_get_property(plyFile, cElementName, &vertexProperties[ 9]); // green
            ply_get_property(plyFile, cElementName, &vertexProperties[10]); // blue
            ply_get_property(plyFile, cElementName, &vertexProperties[11]); // alpha
                                                 
            /* grab all the vertex elements */   
            for ( int vertex = 0; vertex < nElems; vertex++ ) {
                /* grab and element from the file */
                VertexPOD vDesc;
                ply_get_element(plyFile, (void*)&vDesc);

                Vec3Df p(vDesc.x , vDesc.y , vDesc.z );
                Vec3Df n(vDesc.nx, vDesc.ny, vDesc.nz);
                Color  c(vDesc.red, vDesc.green, vDesc.blue, vDesc.alpha);
                Vertex v(p, n, c, vDesc.u, vDesc.v);
                
                m_vertices.push_back(v);
                m_vtxCount++;
                
#ifdef __DEBUG_PLY_READ
                /* print out vertex x,y,z for debugging */
                std::cout << "Read Vertex:" << std::endl;
                std::cout << " - Position = "   << vDesc.x << " " 
                                                << vDesc.y << " " 
                                                << vDesc.z << std::endl;
                std::cout << " - Normal   = "   << vDesc.nx << " " 
                                                << vDesc.ny << " " 
                                                << vDesc.nz << std::endl;
                std::cout << " - Color    = "   << (int)vDesc.red   << " " 
                                                << (int)vDesc.green << " "
                                                << (int)vDesc.blue  << " " 
                                                << (int)vDesc.alpha << std::endl;
                std::cout << " - UVCoords = "   << vDesc.u << ", "
                                                << vDesc.v << std::endl;
#endif
            }
        }

        /* if we're on face elements, read them in */
        if ( elementName.compare("face") == 0 ) {
            /* set up for getting face elements */
            ply_get_property(plyFile, cElementName, &faceProperties[0]);

            /* grab all the face elements */
            for ( int face = 0; face < nElems; face++ ) {
                /* grab an element from the file */
                FacePOD f;
                ply_get_element(plyFile, (void*)&f);

                Face faceObj;
                faceObj.v0 = f.verts[0];
                faceObj.v1 = f.verts[1];
                faceObj.v2 = f.verts[2];

                m_faces.push_back ( faceObj );
                m_faceCount++;

#ifdef __DEBUG_PLY_READ
                /* print out face info, for debugging */
                std::cout << f.nVerts << " ";
                for ( int vert = 0; vert < f.nVerts; vert++ ) {
                    std::cout << f.verts[vert] << " ";
                }
                std::cout << std::endl;
#endif
            }
        }

    }

    /* grab and print out the comments in the file */
    comments = ply_get_comments(plyFile, &nComments);
    for ( int comm = 0; comm < nComments; comm++ ) {
        std::cout << "comment = " << comments[comm] << std::endl; 
    }

    /* grab and print out the object information */
    objectInfo = ply_get_obj_info(plyFile, &nObjectInfo);
    for ( int objInfo = 0; objInfo < nObjectInfo; objInfo++) {
        std::cout << "obj_info = " << objectInfo[objInfo] << std::endl;
    }

    /* close the PLY file */
    ply_close(plyFile);
}

void PointSet::LoadFromFile(const std::string& iFilename) {
    unsigned int plyPos = iFilename.find(".ply");
    if ( (plyPos != std::string::npos) && 
            (plyPos == (iFilename.size() - 4)) ) {
        LoadPlyFile(iFilename.c_str());
    }
}
void PointSet::WriteToFile(const std::string& iFilename) {
    unsigned int plyPos = iFilename.find(".ply");
    if ( (plyPos != std::string::npos) && 
            (plyPos == (iFilename.size() - 4)) ) {
        WritePlyFile(iFilename.c_str());
    }
}

void PointSet::MoveToBarycenter () {
    Vec3Df barycenter ( 0, 0, 0 );
    for ( int vtx = 0; vtx < m_vertices.size (); vtx++ ) {
        const Vertex& v = m_vertices[vtx];

        barycenter += v.GetPosition ();
    }
    barycenter /= m_vertices.size ();
    for ( int vtx = 0; vtx < m_vertices.size (); vtx++ ) {
        Vertex& v = m_vertices[vtx];

        Vec3Df position = v.GetPosition ();
        position -= barycenter;
        v.SetPosition ( position );
    }
}

void PointSet::Draw () const {

    glBegin ( GL_POINTS );
    for ( int vtx = 0; vtx < m_vertices.size (); vtx++ ) {
        const Vertex& v = m_vertices[vtx];

        const Color& c = v.GetColor ();
        const Vec3Df pos = v.GetPosition()/5;

        glColor4f ( c.Red(), c.Green(), c.Blue(), c.Alpha() );
        glVertex3f ( pos[0], pos[1], pos[2] );
    }
    glEnd();

}
