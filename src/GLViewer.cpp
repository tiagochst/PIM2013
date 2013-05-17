#include "GLViewer.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <GL/glut.h>

using namespace std;

GLViewer::GLViewer () : QGLViewer (), noAutoOpenGLDisplayMode(false) {
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

void GLViewer::init() {

    ParameterHandler* params = ParameterHandler::Instance();
    std::string frameID = std::to_string(params -> GetFrame());
    
    std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");
    
    m_frame = Image(RES_IMG_PATH + "image_" + frameID + ".pgm");
    m_depth = Image(RES_IMG_PATH + "depth_" + frameID + ".pgm");

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
    glEnable( GL_CULL_FACE );
    glEnable( GL_POINT_SMOOTH );
    glPointSize( 6.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Add a manipulated frame to the viewer.
    // If you are not "using namespace qglqglviewer", you need
    // to specify: new qglviewer::ManipulatedFrame().
    setManipulatedFrame(new qglviewer::ManipulatedFrame());
    
    restoreStateFromFile();
    
    // Make world axis visible
    // setAxisIsDrawn();
}

static void drawPoints(Image frame, Image depth)
{
    int height = frame.GetHeight();
    int width  = frame.GetWidth();

    glBegin( GL_POINTS );

    for ( int i = 0; i < height; i++ )
    {
        for ( int j = 0; j < width; j++ )
        {
            float greyLvl = frame.GetGreyLvl( i, j );
            float maxGreyLvl = frame.GetMaxGreyLevel();
            float color =  (float)greyLvl/maxGreyLvl;

            glColor3f(color, color, color);
            glVertex3f( (float) j/width,           
                        (float) (height-i)/height, 
                        (float) depth.GetGreyLvl(i,j)/depth.GetMaxGreyLevel());
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

