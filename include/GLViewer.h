#ifndef GLVIEWER_H
#define GLVIEWER_H

#include <GL/glew.h>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMenu>
#include <qcursor.h>
#include <qmap.h>
#include <math.h>
#include <vector>
#include <string>
#include "Image.h"
#include "Config.h"
#include "ParameterHandler.h"


class GLViewer : public QGLViewer  {
    Q_OBJECT
public:
    void reset();
    void init();
    GLViewer ();
    virtual ~GLViewer ();
    
    class Exception  {
    public:
        Exception (const std::string & msg) : message ("[GLViewer]"+msg) {}
        virtual ~Exception () {}
        const std::string & getMessage () const { return message; }
    private:
        std::string message;
    }; 

    bool noAutoOpenGLDisplayMode;
     
protected :
    void draw ();
    QString helpString() const;
    virtual void keyPressEvent(QKeyEvent *e);

 private : 
    Image m_frame;
    Image m_depth;
    bool m_wireframe,;
    bool m_flatShading;
    bool m_mesh;
    bool m_displacements;

};
#endif // GLVIEWER_H
