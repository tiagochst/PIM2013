#include "GLViewer.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <GL/glut.h>

using namespace std;

GLViewer::GLViewer ()
    :   QGLViewer (),
        noAutoOpenGLDisplayMode(false),
        m_frame (),
        m_depth () 
{
}

GLViewer::~GLViewer () {
}

QString GLViewer::helpString() const {
  QString text("<h2>PIM380</h2>");
  text += "A <i>Manipulate face vertes</i>" ;
  return text;
}

// -----------------------------------------------
// Drawing functions
// -----------------------------------------------
void GLViewer::reset() {
     ParameterHandler* params = ParameterHandler::Instance();
     std::string frameID = std::to_string(params -> GetFrame1());
    
     std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");
     m_frame.LoadFromFile(RES_IMG_PATH + "image_" + frameID + ".pgm");
     m_depth.LoadFromFile(RES_IMG_PATH + "depth_" + frameID + ".pgm");
}

void GLViewer::init() {

    ParameterHandler* params = ParameterHandler::Instance();
    std::string frameID = std::to_string(params -> GetFrame1());
    
    std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");
    std::cout << "showing image" <<  frameID << std::endl;
    m_frame.LoadFromFile(RES_IMG_PATH + "image_" + frameID + ".pgm");
   m_depth.LoadFromFile(RES_IMG_PATH + "depth_" + frameID + ".pgm");

  // Swap the CAMERA and FRAME state keys (NoButton and Control)
  // Save CAMERA binding first. See setHandlerKeyboardModifiers() documentation.
#if QT_VERSION < 0x040000
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::AltButton);
    setHandlerKeyboardModifiers(QGLViewer::FRAME,  Qt::NoButton);
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::ControlButton);
#else
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::AltModifier);
    setHandlerKeyboardModifiers(QGLViewer::FRAME,  Qt::NoModifier);
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::ControlModifier);
#endif
    
#ifdef GL_RESCALE_NORMAL  // OpenGL 1.2 Only...
    glEnable(GL_RESCALE_NORMAL);
#endif

    /* Reset far plane*/
    qglviewer::Camera * cam = camera();
    cam ->  setZNearCoefficient(0.000000001f); 
    cam ->  setSceneRadius(10000.0f); 

    glEnable( GL_POINT_SPRITE ); // GL_POINT_SPRITE_ARB if you're
                                 // using the functionality as an extension.
    glDisable( GL_LIGHTING );
    //glEnable( GL_CULL_FACE );
    glEnable( GL_POINT_SMOOTH );
    glPointSize( 1.0 );
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Add a manipulated frame to the viewer.
    setManipulatedFrame(new qglviewer::ManipulatedFrame());

    restoreStateFromFile();
    
}

static void drawPoints (
    const Image& frame,
    const Image& depth
) {
    int height = frame.GetHeight();
    int width = frame.GetWidth();
    float maxGreyLvl = frame.GetMaxGreyLevel ();
    float maxDepth = depth.GetMaxGreyLevel ();

    //glBegin( GL_TRIANGLES );
    glBegin ( GL_POINTS );
    for ( int i = 0; i < height - 1; i++ )
    {
        for ( int j = 0; j < width - 1; j++ )
        {
            float brght;
            
            brght = (float)frame.GetGreyLvl( i    , j     ) / maxGreyLvl;
            float v0c[3]    = { brght, brght, brght };
            float v0x       = (float)(          j     ) / width;
            float v0y       = (float)( height - i     ) / height;
            float v0z       = (float)( depth.GetGreyLvl ( i    , j     ) ) / maxDepth;

            //brght = (float)frame.GetGreyLvl( i    , j + 1 ) / maxGreyLvl;
            //float v1c[3]    = { brght, brght, brght };
            //float v1x       = (float)(          j + 1 ) / width;
            //float v1y       = (float)( height - i     ) / height;
            //float v1z       = (float)( depth.GetGreyLvl ( i    , j + 1 ) ) / maxDepth;

            //brght = (float)frame.GetGreyLvl( i + 1, j     ) / maxGreyLvl;
            //float v2c[3]    = { brght, brght, brght };
            //float v2x       = (float)(          j     ) / width;
            //float v2y       = (float)( height - i - 1 ) / height;
            //float v2z       = (float)( depth.GetGreyLvl ( i + 1, j     ) ) / maxDepth;

            //brght = (float)frame.GetGreyLvl( i + 1, j + 1 ) / maxGreyLvl;
            //float v3c[3]    = { brght, brght, brght };
            //float v3x       = (float)(          j + 1 ) / width;
            //float v3y       = (float)( height - i - 1 ) / height;
            //float v3z       = (float)( depth.GetGreyLvl ( i + 1, j + 1 ) ) / maxDepth;

            glColor3fv ( v0c );
            glVertex3f ( v0x, v0y, v0z );

            //glColor3fv ( v2c );
            //glVertex3f ( v2x, v2y, v2z );

            //glColor3fv ( v1c );
            //glVertex3f ( v1x, v1y, v1z );

            //glColor3fv ( v1c );
            //glVertex3f ( v1x, v1y, v1z );

            //glColor3fv ( v2c );
            //glVertex3f ( v2x, v2y, v2z );

            //glColor3fv ( v3c );
            //glVertex3f ( v3x, v3y, v3z );
        }
    }
    glEnd();
}

void GLViewer::draw () {
  // Here we are in the world coordinate system.
  // Draw your scene here.
  // Save the current model view matrix (not needed here in fact)
  glPushMatrix();

  // Multiply matrix to get in the frame coordinate system.
  glMultMatrixd(manipulatedFrame()->matrix());

  // Scale down the drawings
  glScalef(0.3f, 0.3f, 0.3f);

  // Draw an axis using the QGLViewer static function
  drawAxis();

  // Draws the image in 3D.
  drawPoints(m_frame,m_depth);
  
  // Restore the original (world) coordinate system
  glPopMatrix();
}

