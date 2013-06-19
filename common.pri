#-------------------------------------------------
#
# Project created by QtCreator 2013-05-13T18:31:08
#
#-------------------------------------------------
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
                        include/Tools.h                 \
                        include/FileWriterServices.h    \
                        include/PPMImage.h              \
                        include/PNMImage.h              \
                        include/PNMImageDefs.h          \
                        include/PixelTracker.h          \
                        include/ImagePyramid.h          \
                        include/Frame.h                 \
                        include/Clip.h                  \
                        include/TrackInfo.h

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
                        src/ParameterHandler.cpp        \
                        src/PlyFile.c                   \
                        src/FileWriterServices.cpp      \
                        src/PPMImage.cpp                \
                        src/PNMImage.cpp                \
                        src/PixelTracker.cpp            \
                        src/ImagePyramid.cpp            \
                        src/Frame.cpp                   \
                        src/Clip.cpp                    \
                        src/TrackInfo.cpp 


MOC_DIR              =  .tmp
OBJECTS_DIR          =  .tmp
DESTDIR              =  ./bin/

QMAKE_CFLAGS        +=  -fopenmp -ggdb
QMAKE_CXXFLAGS      +=  -std=c++0x -fopenmp -ggdb
INCLUDEPATH         +=  './include'                     \ 
                        './external/common'     

DEFINES             +=  TRACK_REGULATION
#DEFINES             +=  __DEBUG_PLY_READ 
DEFINES             +=  IMG_EQ_SIMG
unix {
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

    LIBS            +=  -lQGLViewerd2

}
win32:release{

    LIBS            +=  -lQGLViewer2

}

FORMS +=
