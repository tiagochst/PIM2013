#include "Frame.h"

#include "PointSet.h"
#include "PPMImage.h"
#include "Image.h"
#include "Color.h"
#include "Vec3D.hpp"
#include <GL/gl.h>
#include <GL/glut.h>


Frame::Frame () 
    :   m_mesh ( 0x0 ),
        m_texture ( 0x0 ),
        m_depthMap ( 0x0 ),
        m_displacements ( 0x0 ),
        m_rawDisplacementsX (),
        m_rawDisplacementsY (),
        m_rawDisplacementsZ ()
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
    if ( m_displacements ) {
        delete m_displacements;
        m_displacements = (PPMImage*)0x0;
    }
}

void Frame::Draw () const {
    DrawMesh ();
    if ( m_displacements ) {
        DrawDisplacements ();
    }
}

void Frame::DrawMesh () const {
    glLineWidth ( 1.0f );
    glPolygonMode ( GL_FRONT_AND_BACK, GL_LINE );
    glBegin ( GL_TRIANGLES );
    for ( unsigned int face = 0; face < m_mesh->GetNumFaces (); face++ ) {
        const Face& f = m_mesh->GetFace ( face );

        const Vertex&   v0  =   m_mesh->GetVertex ( f.v0 );
        const Color&    c0  =   v0.GetColor ();
        const Vec3Df   p0  =   v0.GetPosition ();

        const Vertex&   v1  =   m_mesh->GetVertex ( f.v1 );
        const Color&    c1  =   v1.GetColor ();
        const Vec3Df   p1  =   v1.GetPosition ();

        const Vertex&   v2  =   m_mesh->GetVertex ( f.v2 );
        const Color&    c2  =   v2.GetColor ();
        const Vec3Df   p2  =   v2.GetPosition ();

        if ( p0[2] == 0.0f || p1[2] == 0.0f || p2[2] == 0.0f ) {
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
    glLineWidth ( 2.5f );
    
    glShadeModel ( GL_SMOOTH );
    glBegin ( GL_LINES );
    float maxVal = m_displacements->GetMaxValue ();
    for ( unsigned int vtx = 0; vtx < m_mesh->GetNumVertices (); vtx++ ) {
        const Vertex& vert = m_mesh->GetVertex ( vtx );

        unsigned int u,v;
        vert.GetUVCoord ( u, v );

        float r = (float)m_displacements->GetChannelValue ( v, u, RED   ) / maxVal;
        float g = (float)m_displacements->GetChannelValue ( v, u, GREEN ) / maxVal;
        float b = (float)m_displacements->GetChannelValue ( v, u, BLUE  ) / maxVal;

        Color c ( r, g, b );
        
        Vec3Df disp = Vec3Df (
            m_rawDisplacementsX ( v, u ),
            m_rawDisplacementsY ( v, u ),
            m_rawDisplacementsZ ( v, u )
        );
        Vec3Df pos0 = vert.GetPosition ();
        Vec3Df pos1 = pos0 + disp / 5.0f;

        if ( fabs ( disp[2] )  >= 1000.0f ) {
            continue;
        }
        if ( pos0[2]  > -500.0f || pos0[2]  > -500.0f || pos0[2]  > -500.0f ) {
            continue;
        }
        if ( pos1[2]  > -500.0f || pos1[2]  > -500.0f || pos1[2]  > -500.0f ) {
            continue;
        }

        glColor3f ( r, g, b );
        glVertex3f ( pos0[0], pos0[1], pos0[2] );

        glVertex3f ( pos1[0], pos1[1], pos1[2] );
    }
    glEnd ();
}

void Frame::LoadDisplacements ( const std::string& iPath ) {
    if ( m_displacements ) {
        delete m_displacements;
        m_displacements = (PPMImage*)0x0;
    }
    m_displacements = PPMImage::TryLoadFromFile ( iPath + "displacement.ppm" );

    m_rawDisplacementsX.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_rawDisplacementsY.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );
    m_rawDisplacementsZ.resize (
        m_texture->GetHeight (),
        m_texture->GetWidth ()
    );

    std::ifstream rawDisps ( (iPath + "rawDisplacement.dat").c_str(), std::ifstream::binary );
    if ( rawDisps.good () && rawDisps.is_open () ) {
        for ( unsigned int x = 0; x < m_texture->GetWidth (); x++ ) {
            for ( unsigned int y = 0; y < m_texture->GetHeight (); y++ ) {
                float rdx, rdy, rdz;

                rawDisps.read ( (char*)&rdx, sizeof ( float ) );
                rawDisps.read ( (char*)&rdy, sizeof ( float ) );
                rawDisps.read ( (char*)&rdz, sizeof ( float ) );

                m_rawDisplacementsX ( y, x ) = rdx;
                m_rawDisplacementsY ( y, x ) = rdy;
                m_rawDisplacementsZ ( y, x ) = rdz;
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

