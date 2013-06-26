#ifndef _FRAME_H_
#define _FRAME_H_

#include <Eigen/Dense>

class PointSet;
class Image;
class PPMImage;
class PointSet;

class Frame {
private:
    PointSet*           m_mesh;
    Image*              m_texture;
    Image*              m_depthMap;
    PPMImage*           m_displacements;
    Eigen::MatrixXf     m_rawDisplacementsX;
    Eigen::MatrixXf     m_rawDisplacementsY;
    Eigen::MatrixXf     m_rawDisplacementsZ;

public:
    Frame ();
    ~Frame ();

    void DrawMesh () const;
    void DrawDisplacements () const;
    void Draw () const;

    void LoadFromFile ( const std::string& iPath );
    void LoadDisplacements ( const std::string& iPath );

    const PointSet* GetMesh () const {
        return m_mesh; 
    }
    PointSet* GetMesh () {
        return m_mesh; 
    }
    void SetMesh ( PointSet* iMesh ) {
        if ( m_mesh ) delete m_mesh;
        m_mesh = iMesh; 
    }

    const Image* GetTexture () const {
        return m_texture; 
    }
    Image* GetTexture () {
        return m_texture; 
    }
    void SetTexture ( Image* iTexture ) {
        if ( m_texture ) delete m_texture;
        m_texture = iTexture; 
    }

    const Image* GetDepthMap () const {
        return m_depthMap; 
    }
    Image* GetDepthMap () {
        return m_depthMap; 
    }
    void SetDepthMap ( Image* iDepthMap ) {
        if ( m_depthMap ) delete m_depthMap;
        m_depthMap = iDepthMap; 
    }

    const PPMImage* GetDisplacements () const {
        return m_displacements; 
    }
    PPMImage* GetDisplacements () {
        return m_displacements; 
    }
    void SetDisplacements ( PPMImage* iDisplacements ) {
        if ( m_displacements ) delete m_displacements;
        m_displacements = iDisplacements; 
    }
};

#endif // _FRAME_H_