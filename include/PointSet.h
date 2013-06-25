#ifndef _POINTSET_H_
#define _POINTSET_H_

#include <cstring>
#include <vector>
#include "Vertex.h"
#include <map>
#include "MathUtils.h"
#include <XnTypes.h>
#include <Eigen/Dense>

class Image;
class PPMImage;

struct Face {
    unsigned int v0, v1, v2;
};

class PointSet {
private:
    unsigned int                                    m_vtxCount;
    unsigned int                                    m_faceCount;
    std::vector<Vertex>                             m_vertices;
    std::vector<Face>                               m_faces;
    std::map<CartCoordf, unsigned int>              m_imageToRealWorld;
    
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
    void RecalculateNormals ();

    void Draw () const;

    const Vertex& GetVertex ( const unsigned int& iVertex ) const;
    Vertex& GetVertex ( const unsigned int& iVertex );
    unsigned int& GetVertexId ( const float& iU, const float& iV );
    const Face& GetFace ( const unsigned int& iFace ) const;
    Face& GetFace ( const unsigned int& iFace );

    const unsigned int& GetNumVertices () const;
    const unsigned int& GetNumFaces () const;

    void BuildFromKinectPointCloud (
        const PPMImage&           iColorData,
        const Image&              iFilteredDepth,
        std::vector<XnPoint3D>&   iRealWorld
    );
    void Build (
        const PPMImage& iColorData,
        const Image&    iRawDepth,
        const Image&    iFilteredDepth
    );
    void ApplyMotionField (
        const Eigen::MatrixXf&  iDeltaU,
        const Eigen::MatrixXf&  iDeltaV,
        const Eigen::MatrixXf&  iDeltaX,
        const Eigen::MatrixXf&  iDeltaY,
        const Eigen::MatrixXf&  iDeltaZ
    );
};

#endif
