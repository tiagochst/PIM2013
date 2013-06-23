#include "GLViewer.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <string>
#include <GL/glut.h>
#include "Camera.h"

#include "Frame.h"

using namespace std;

GLViewer::GLViewer ()
    :   QGLViewer (),
        noAutoOpenGLDisplayMode(false),
        m_frame (),
        m_depth (),
        m_mesh(true),
        m_displacements(true)
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
    
     std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/f" + frameID + "/");
     m_frame.LoadFromFile(RES_IMG_PATH + "texture.pgm");
     m_depth.LoadFromFile(RES_IMG_PATH + "depthMap.pgm");
}

void GLViewer::init() {


    /* 
       Keyboard shortcut customization           
       Changes standard action key bindings       
    */

    /* Define 'Control+Q' as the new exit shortcut (default was 'Escape')*/
    setShortcut(EXIT_VIEWER, Qt::CTRL+Qt::Key_Q);
    
    /* Set 'Control+F' as the FPS toggle state key.*/
    setShortcut(DISPLAY_FPS, Qt::CTRL+Qt::Key_F);
    
    /* Disable draw grid toggle shortcut (default was 'G')*/
    setShortcut(DRAW_GRID, 0);
        
    /* Add custom key description (see keyPressEvent).*/
    setKeyDescription(Qt::Key_W, "Toggles wire frame display");
    setKeyDescription(Qt::Key_F, "Toggles flat shading display");

    /* Add custom mouse bindings description (see mousePressEvent()) */
    setMouseBindingDescription(Qt::RightButton, "Opens a camera path context menu");

    ParameterHandler* params = ParameterHandler::Instance();
    std::string frameID = std::to_string(params -> GetFrame1());
     std::string RES_IMG_PATH(Config::OutputPath() + "CapturedFrames/f" + frameID + "/");
     m_frame.LoadFromFile(RES_IMG_PATH + "texture.pgm");
     m_depth.LoadFromFile(RES_IMG_PATH + "depthMap.pgm");

    // Swap the CAMERA and FRAME state keys (NoButton and Control)
    // Save CAMERA binding first. See setHandlerKeyboardModifiers() documentation.
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::AltModifier);
    setHandlerKeyboardModifiers(QGLViewer::FRAME,  Qt::NoModifier);
    setHandlerKeyboardModifiers(QGLViewer::CAMERA, Qt::ControlModifier);

#ifdef GL_RESCALE_NORMAL  // OpenGL 1.2 Only...
    glEnable(GL_RESCALE_NORMAL);
#endif
    
    /* Reset far plane*/
    qglviewer::Camera * cam = camera();
    cam -> setZNearCoefficient(0.000000001f); 
    cam -> setSceneRadius(100000000.0f); 

    glEnable( GL_POINT_SPRITE );    // GL_POINT_SPRITE_ARB if you're
                                    // using the functionality as an extension.
    glDisable( GL_LIGHTING );
    glEnable( GL_POINT_SMOOTH );
    glPointSize( 1.0 );

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Add a manipulated frame to the viewer.
    setManipulatedFrame(new qglviewer::ManipulatedFrame());

    //restoreStateFromFile();

    m_wireframe = false;
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Make world axis visible
    //setAxisIsDrawn();
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

#include <XnTypes.h>
#include "PPMImage.h"
#include "Image.h"
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

    ParameterHandler* params = ParameterHandler::Instance ();
    if ( params->GetCamera () ) {
        
        Camera& cam = Camera::Instance ();
        PointSet ps;
        cam.ReadFrame (
            (Image*)0x0,
            (Image*)0x0,
            &ps
        );

        ps.MoveToBarycenter ();
        ps.Draw ();
    } else {
        const Frame* f = params->GetCurrentFrame ();
        
        if ( f )
            f->Draw ();
    }
    
    // Draws the image in 3D.
    //drawPoints(m_frame,m_depth);
    
    // Restore the original (world) coordinate system
    glPopMatrix();
}



void GLViewer::keyPressEvent(QKeyEvent *e)
{
    ParameterHandler* params = ParameterHandler::Instance ();
    const Qt::KeyboardModifiers modifiers = e->modifiers();
    
    bool handled = false;
    if ((e->key()==Qt::Key_W) && (modifiers==Qt::NoButton))
        {
            if (m_wireframe) {
                m_wireframe = false;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            } else {
                m_wireframe = true;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            handled = true;
            updateGL();
        }
    else
        if ((e->key()==Qt::Key_F) && (modifiers==Qt::NoButton))
            {
                m_flatShading = !m_flatShading;
                if (m_flatShading)
                    glShadeModel(GL_FLAT);
                else
                    glShadeModel(GL_SMOOTH);
                handled = true;
                updateGL();
            }
        else
            if ((e->key()==Qt::Key_D) && (modifiers==Qt::NoButton))
                {
                    m_displacements = !m_displacements;
                    params -> SetDrawDisplacement(m_displacements);
                    handled = true;
                    updateGL();
                }
        else
            if ((e->key()==Qt::Key_M) && (modifiers==Qt::NoButton))
                {
                    m_mesh = !m_mesh;
                    params -> SetDrawMesh(m_mesh);
                    handled = true;
                    updateGL();
                }
    
    if (!handled)
        QGLViewer::keyPressEvent(e);
}

