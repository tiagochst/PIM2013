#include "Frame.h"

#include "PointSet.h"
#include "PPMImage.h"
#include "Image.h"
#include "Color.h"
#include "Vec3D.hpp"
#include <GL/gl.h>
#include <GL/glut.h>
#include "ParameterHandler.h"

Frame::Frame () 
    :   m_mesh ( 0x0 ),
        m_texture ( 0x0 ),
        m_depthMap ( 0x0 ),
        m_disparityMap ( 0x0 ),
        m_motionFieldX (),
        m_motionFieldY (),
        m_motionFieldZ ()
{}

Frame::~Frame ()
{
    if ( m_mesh ) {
        delete m_mesh;
        m_mesh = (PointSet*)0x0;
    }
    if ( m_texture ) {
        delete m_texture;
        m_texture = (Image*)0x0;
    }
    if ( m_depthMap ) {
        delete m_depthMap;
        m_depthMap = (Image*)0x0;
    }
    if ( m_disparityMap ) {
        delete m_disparityMap;
        m_disparityMap = (PPMImage*)0x0;
    }
}

void Frame::Draw () const {
    ParameterHandler* params = ParameterHandler::Instance ();

    //std::cout << "Draw disp:" << params -> GetDrawDisplacement() << std::endl;
    //std::cout << "Draw mesh:" << params -> GetDrawMesh() << std::endl;

    if ( params -> GetDrawMesh() ) {
        DrawMesh ();
    }
    if ( params -> GetDrawDisplacement()){ 
        if(m_disparityMap) {
            DrawDisplacements ();
        }
    }
}

void Frame::DrawMesh () const {
    ParameterHandler* params = ParameterHandler::Instance ();
    const float n = -params->GetNearPlane ();
    const float f = -params->GetFarPlane ();

    glLineWidth ( 1.0f );

    // Default is used from class glviewer.
    //glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );

    glBegin ( GL_TRIANGLES );
    for ( unsigned int face = 0; face < m_mesh->GetNumFaces (); face++ ) {
        const Face& fc = m_mesh->GetFace ( face );

        const Vertex&   v0  =   m_mesh->GetVertex ( fc.v0 );
        const Color&    c0  =   v0.GetColor ();
        const Vec3Df   p0   =   v0.GetPosition ();

        const Vertex&   v1  =   m_mesh->GetVertex ( fc.v1 );
        const Color&    c1  =   v1.GetColor ();
        const Vec3Df   p1   =   v1.GetPosition ();

        const Vertex&   v2  =   m_mesh->GetVertex ( fc.v2 );
        const Color&    c2  =   v2.GetColor ();
        const Vec3Df   p2   =   v2.GetPosition ();

        if (
               ( p0[2] >= n ) || ( p0[2] <= f )
            || ( p1[2] >= n ) || ( p1[2] <= f )
            || ( p2[2] >= n ) || ( p2[2] <= f )
        ) {
            //std::cout << p0 << std::endl;
            //std::cout << p1 << std::endl;
            //std::cout << p2 << std::endl;
            continue;
        }

        glColor4f ( c0.Red(), c0.Green(), c0.Blue(), c0.Alpha() );
        glVertex3f ( p0[0], p0[1], p0[2] );

        glColor4f ( c1.Red(), c1.Green(), c1.Blue(), c1.Alpha() );
        glVertex3f ( p1[0], p1[1], p1[2] );
        
        glColor4f ( c2.Red(), c2.Green(), c2.Blue(), c2.Alpha() );
        glVertex3f ( p2[0], p2[1], p2[2] );
    }
    glEnd ();
    //m_mesh->Draw ();
}

void Frame::DrawDisplacements () const {
    ParameterHandler* params = ParameterHandler::Instance ();
    const float n = -params->GetNearPlane ();
    const float f = -params->GetFarPlane ();

    glLineWidth ( 2.5f );
    
    glShadeModel ( GL_SMOOTH );
    glBegin ( GL_LINES );
    float maxVal = m_disparityMap->GetMaxValue ();
    for ( unsigned int vtx = 0; vtx < m_mesh->GetNumVertices (); vtx++ ) {
        const Vertex& vert = m_mesh->GetVertex ( vtx );

        float u,v;
        vert.GetUVCoord ( u, v );

        float r = (float)m_disparityMap->GetChannelValue ( v, u, RED   ) / maxVal;
        float g = (float)m_disparityMap->GetChannelValue ( v, u, GREEN ) / maxVal;
        float b = (float)m_disparityMap->GetChannelValue ( v, u, BLUE  ) / maxVal;

        Color c ( r, g, b );
        
        Vec3Df disp = Vec3Df (
            m_motionFieldX ( v, u ),
            m_motionFieldY ( v, u ),
            m_motionFieldZ ( v, u )
        );
        Vec3Df pos0 = vert.GetPosition ();
        Vec3Df pos1 = pos0 + disp;

        if (
               ( pos0[2] >= n ) || ( pos0[2] <= f )
            || ( pos1[2] >= n ) || ( pos1[2] <= f )
        ) {
            //std::cout << p0 << std::endl;
            //std::cout << p1 << std::endl;
            //std::cout << p2 << std::endl;
            continue;
        }

        glColor3f ( r, g, b );
        glVertex3f ( pos0[0], pos0[1], pos0[2] );

        glVertex3f ( pos1[0], pos1[1], pos1[2] );
    }
    glEnd ();
}

void Frame::LoadMotionField ( const std::string& iPath ) {
    if ( m_disparityMap ) {
        delete m_disparityMap;
        m_disparityMap = (PPMImage*)0x0;
    }
    m_disparityMap = PPMImage::TryLoadFromFile ( iPath + "disparityMap.ppm" );

    m_motionFieldU.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_motionFieldV.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_motionFieldX.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_motionFieldY.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_motionFieldZ.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );

    std::ifstream motionField ( (iPath + "motionField.dat").c_str(), std::ifstream::binary );
    if ( motionField.good () && motionField.is_open () ) {
        for ( unsigned int x = 0; x < m_texture->GetWidth (); x++ ) {
            for ( unsigned int y = 0; y < m_texture->GetHeight (); y++ ) {
                float rdu, rdv, rdx, rdy, rdz;

                motionField.read ( (char*)&rdu, sizeof ( float ) );
                motionField.read ( (char*)&rdv, sizeof ( float ) );
                motionField.read ( (char*)&rdx, sizeof ( float ) );
                motionField.read ( (char*)&rdy, sizeof ( float ) );
                motionField.read ( (char*)&rdz, sizeof ( float ) );

                m_motionFieldU ( y, x ) = rdu;
                m_motionFieldV ( y, x ) = rdv;
                m_motionFieldX ( y, x ) = rdx;
                m_motionFieldY ( y, x ) = rdy;
                m_motionFieldZ ( y, x ) = rdz;
            }
        }
    }
}

void Frame::LoadFromFile ( const std::string& iPath ) {
    if ( m_mesh ) {
        delete m_mesh;
        m_mesh = (PointSet*)0x0;
    }
    m_mesh = new PointSet ();
    m_mesh->LoadFromFile ( iPath + "mesh.ply" );

    if ( m_texture ) {
        delete m_texture;
        m_texture = (Image*)0x0;
    }
    m_texture = new Image ();
    m_texture->LoadFromFile ( iPath + "texture.pgm" );

    if ( m_depthMap ) {
        delete m_depthMap;
        m_depthMap = (Image*)0x0;
    }
    m_depthMap = new Image ();
    m_depthMap->LoadFromFile ( iPath + "depthMap.pgm" ); 
}

void Frame::ApplyMotionField (
    PointSet&       ioMesh
) const {
    ioMesh.ApplyMotionField (
        m_motionFieldU, 
        m_motionFieldV, 
        m_motionFieldX, 
        m_motionFieldY, 
        m_motionFieldZ
    );
}

