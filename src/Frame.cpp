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
    //glBegin ( GL_TRIANGLES );
    //for ( unsigned int face = 0; face < m_mesh->GetNumFaces (); face++ ) {
    //    const Face& f = m_mesh->GetFace ( face );

    //    const Vertex&   v0  =   m_mesh->GetVertex ( f.v0 );
    //    const Color&    c0  =   v0.GetColor ();
    //    const Vec3Df   p0  =   v0.GetPosition() / 32000;

    //    const Vertex&   v1  =   m_mesh->GetVertex ( f.v1 );
    //    const Color&    c1  =   v1.GetColor ();
    //    const Vec3Df   p1  =   v1.GetPosition() / 32000;

    //    const Vertex&   v2  =   m_mesh->GetVertex ( f.v2 );
    //    const Color&    c2  =   v2.GetColor ();
    //    const Vec3Df   p2  =   v2.GetPosition() / 32000;

    //    glColor4f ( c0.Red(), c0.Green(), c0.Blue(), c0.Alpha() );
    //    glVertex3f ( p0[0], p0[1], p0[2] );

    //    glColor4f ( c1.Red(), c1.Green(), c1.Blue(), c1.Alpha() );
    //    glVertex3f ( p1[0], p1[1], p1[2] );
    //    
    //    glColor4f ( c2.Red(), c2.Green(), c2.Blue(), c2.Alpha() );
    //    glVertex3f ( p2[0], p2[1], p2[2] );
    //}
    //glEnd ();
    m_mesh->Draw ();
}

void Frame::DrawDisplacements () const {
    glLineWidth ( 2.5f );
    glBegin ( GL_LINES );
    for ( unsigned int vtx = 0; vtx < m_mesh->GetNumVertices (); vtx++ ) {
        const Vertex& vert = m_mesh->GetVertex ( vtx );

        unsigned int u,v;
        vert.GetUVCoord ( u, v );

        float maxVal = m_displacements->GetMaxValue ();
        float r = m_displacements->GetChannelValue ( v, u, RED   ) / maxVal;
        float g = m_displacements->GetChannelValue ( v, u, GREEN ) / maxVal;
        float b = m_displacements->GetChannelValue ( v, u, BLUE  ) / maxVal;

        Color c ( r, g, b );
        
        float hue, sat, val;
        c.ToHSV ( hue, sat, val );
        hue *= M_PI / 180.0f;

        Vec3Df disp = Vec3Df::polarToCartesian ( Vec3Df ( val, M_PI_2, -hue ) );

        Vec3Df pos = vert.GetPosition ()/5;
        const Color& col = vert.GetColor ();
        glColor3f ( col.Red(), col.Green(), col.Blue() );
        glVertex3f ( pos[0], pos[1], pos[2] );

        pos += disp / 2;
        glVertex3f ( pos[0], pos[1], pos[2] );

        glColor3f ( r, g, b );
        glVertex3f ( pos[0], pos[1], pos[2] );

        pos += disp / 2;
        glVertex3f ( pos[0], pos[1], pos[2] );
    }
    glEnd ();
}
