#-------------------------------------------------
#
# Project created by QtCreator 2013-05-13T18:31:08
#
#-------------------------------------------------
TEMPLATE             =  app
TARGET               =  main
CONFIG              +=  qt opengl warn_off xml console debug_and_release thread
QT                  +=  core gui
QT                  *=  opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS              =  include/Window.h                \ 
                        include/GLViewer.h              \ 
                        include/Camera.h                \
                        include/Image.h                 \ 
                        include/ImageBase.h             \ 
                        include/SubImage.h              \ 
                        include/Color.h                 \ 
                        include/PlyFile.h               \ 
                        include/Config.h                \ 
                        include/PointSet.h              \ 
                        include/ParameterHandler.h      \
                        include/Vertex.h                \     
                        include/Writer.h                \
                        include/Tools.h

SOURCES              =  src/Window.cpp                  \
                        src/GLViewer.cpp                \
                        src/Camera.cpp                  \ 
                        src/Image.cpp                   \ 
                        src/ImageBase.cpp               \ 
                        src/SubImage.cpp                \ 
                        src/Vertex.cpp                  \ 
                        src/Color.cpp                   \ 
                        src/Config.cpp                  \ 
                        src/PointSet.cpp                \
                        src/main.cpp                    \
                        src/ParameterHandler.cpp        \
                        src/PlyFile.c

MOC_DIR              =  .tmp
OBJECTS_DIR          =  .tmp

QMAKE_CFLAGS        +=  -fopenmp
QMAKE_CXXFLAGS      +=  -std=c++0x -fopenmp
INCLUDEPATH         +=  './include'                     \ 
                        './external/common'     
unix {
    DESTDIR          =  ./bin/linux/

    LIBS            +=  -lGLEW                          \
                        -lQGLViewer                     \
                        -lgomp                          \
                        -lglut                          \
                        -lm                             \
                        -lOpenNI                        \
                        -lusb-1.0                       \
                        -lGLU                           \
                        -lGL

    INCLUDEPATH     +=  './external/linux'              \
                        '/usr/include/libusb-1.0'       \ 
                        '/usr/include/libdrm'           \ 
                        '/usr/include/ni/'              \ 
                        '/usr/include/openni'
}

win32 {

    QMAKE_LIBDIR    +=  '$(OPENNI_NI_LIB64)'            \
                        './lib/win32'
    
    DESTDIR          =  ./bin/win32

    LIBS            +=  -lopenNI64                      \
                        -lopengl32                      \
                        -lglut32                        \
                        -lglew32                        \
                        -lgomp                          \
                        -lm

    INCLUDEPATH     +=  './external/win32'              \
                        '$(OPENCV_DIR)include'          \
                        '$(OPENNI_NI_INCLUDE)'

}
win32:debug {

    DESTDIR          =  ./bin/win32/debug
    
    LIBS            +=  -lQGLViewerd2

}
win32:release{

    DESTDIR          =  ./bin/win32/release

    LIBS            +=  -lQGLViewer2

}
