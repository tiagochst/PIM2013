#-------------------------------------------------
#
# Project created by QtCreator 2013-05-13T18:31:08
#
#-------------------------------------------------
TEMPLATE = app
TARGET   = main
CONFIG  += qt opengl  warn_off xml console release thread
QT      += core gui
QT *= opengl xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

FORMS *= resources/Project/Ui/mainInterface.Qt4.ui

HEADERS =   include/Window.h    \ 
            include/GLViewer.h  \ 
            include/Camera.h    \
            include/Image.h     \ 
            include/Color.h     \ 
            include/PlyFile.h   \ 
            include/Config.h    \ 
            include/PointSet.h  \ 
            include/Vertex.h    \
            ui_mainInterface.Qt4.h

SOURCES = src/Window.cpp    \
          src/GLViewer.cpp  \
          src/Camera.cpp    \ 
          src/Image.cpp     \ 
          src/Vertex.cpp    \ 
          src/Color.cpp     \ 
          src/Config.cpp    \ 
          src/PointSet.cpp  \
          src/main.cpp      \
          src/PlyFile.c

DESTDIR=./bin

unix {

  QMAKE_CFLAGS   += -O3 -fopenmp `pkg-config --cflags openni glu`

  QMAKE_CXXFLAGS += -O3 -fopenmp `pkg-config --cflags openni glu`


  LIBS += -lGLEW \
        -lQGLViewer \
        -lgomp \
        -lglut \
        -lm    \
        `pkg-config --libs openni glu`

  INCLUDEPATH += './include'                \ 
                 './external'               \ 
                 '/usr/include/libusb-1.0'  \ 
                 '/usr/include/libdrm'      \ 
                 '/usr/include/ni/'         \ 
                 '/usr/include/openni'
}

MOC_DIR = .tmp
OBJECTS_DIR = .tmp
