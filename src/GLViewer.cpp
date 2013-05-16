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

//gluPerspective(M_PI_4,
//                   640.0f/480.0f,
//                   1.0f,10000000.0f);
    
       qglviewer::Camera * cam = camera();

cam ->  setZNearCoefficient(0.000000001f); 
cam ->  setSceneRadius(10000.0f); 

std::cout << cam -> sceneRadius () << std::endl; 
std::cout << cam -> zNearCoefficient () << std::endl; 

    glEnable( GL_POINT_SPRITE ); // GL_POINT_SPRITE_ARB if you're
                                 // using the functionality as an extension.

    glDisable( GL_LIGHTING );
    glEnable( GL_CULL_FACE );
    glEnable( GL_POINT_SMOOTH );
//    glEnable( GL_BLEND );
//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPointSize( 6.0 );
    
//    glDepthMask( GL_TRUE );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Add a manipulated frame to the viewer.
    // If you are not "using namespace qglqglviewer", you need
    // to specify: new qglviewer::ManipulatedFrame().
    setManipulatedFrame(new qglviewer::ManipulatedFrame());
    
    restoreStateFromFile();
    
    // Make world axis visible
    // setAxisIsDrawn();
}



static void drawSpiral()
{
  const float nbSteps = 200.0;
  glBegin(GL_QUAD_STRIP);
  for (float i=0; i<nbSteps; ++i)
    {
      float ratio = i/nbSteps;
      float angle = 21.0*ratio;
      float c = cos(angle);
      float s = sin(angle);
      float r1 = 1.0 - 0.8*ratio;
      float r2 = 0.8 - 0.8*ratio;
      float alt = ratio - 0.5;
      const float nor = .5;
      const float up = sqrt(1.0-nor*nor);
      glColor3f(1.0-ratio, 0.2f , ratio);
      glNormal3f(nor*c, up, nor*s);
      glVertex3f(r1*c, alt, r1*s);
      glVertex3f(r2*c, alt+0.05, r2*s);
    }
  glEnd();
}

static void drawPoints()
{
    static const std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/");

    static Image frame(RES_IMG_PATH + "image_0.pgm");
    static Image depth(RES_IMG_PATH + "depth_0.pgm");
    int height = frame.GetHeight();
    int width = frame.GetWidth();

    glBegin( GL_POINTS );

    for ( int i = 0; i < height; i++ )
    {
        for ( int j = 0; j < width; j++ )
        {
            float greyLvl = frame.GetGreyLvl( i, j );
            float maxGreyLvl = frame.GetMaxGreyLevel();
            float color =  (float)greyLvl/maxGreyLvl;

            glColor3f(color, color, color);
glVertex3f( (float)j/width, (height-(float)i)/height, 
                        (float)depth.GetGreyLvl(i,j)/depth.GetMaxGreyLevel());

        }
    }

    glEnd();
//    glFinish();
//  glutSwapBuffers();
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

  // Draws a frame-related spiral.
  //drawSpiral();
  drawPoints();
  
  // Restore the original (world) coordinate system
  glPopMatrix();
}

