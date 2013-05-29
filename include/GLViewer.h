#ifndef GLVIEWER_H
#define GLVIEWER_H

#include <GL/glew.h>
#include <QGLViewer/qglviewer.h>
#include <QGLViewer/camera.h>
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

 private : 
    Image m_frame;
    Image m_depth;

};
#endif // GLVIEWER_H
