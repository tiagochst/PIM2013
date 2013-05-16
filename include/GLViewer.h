#ifndef GLVIEWER_H
#define GLVIEWER_H

#include <GL/glew.h>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
#include <vector>
#include <string>
#include "Image.h"
#include "Config.h"


class GLViewer : public QGLViewer  {
    Q_OBJECT
public:
    inline void reset(){init();}
    
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
     
    //public slots :
    
protected :
    void init();
    void draw ();
    QString helpString() const;

    /* virtual void keyPressEvent (QKeyEvent * event); */
    /* virtual void keyReleaseEvent (QKeyEvent * event); */
    /* virtual void mousePressEvent (QMouseEvent * event); */
    /* virtual void wheelEvent (QWheelEvent * e); */

private:

};
#endif // GLVIEWER_H
