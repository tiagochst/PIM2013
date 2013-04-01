# additional include paths necessary to compile the C++ programs
# Debug Flags
if DEBUG
AM_CPPFLAGS=-g -O0
AM_CXXFLAGS=-D__DEBUG_PLY_READ
else
AM_CFLAGS = -O3 -msse2
AM_CXXFLAGS =-O3 -msse2
AM_CPPFLAGS =-O3 -DNDEBUG 
endif

# Include flags
AM_CXXFLAGS += -I/usr/include/ni/ -I/usr/include/opencv/ -I/usr/include/opencv2/ -lOpenNI -lm -lglut $(GLU_CFLAGS) $(GLU_LIBS) -lm -g $(OPENCV_CFLAGS) $(OPENCV_LIBS) -Wno-write-strings

AM_CPPFLAGS += -I$(srcdir)/../include -I$(srcdir)/../external 


###############################################################################
# THE PROGRAMS TO BUILD
###############################################################################
bin_PROGRAMS = main
main_header = \
		Color.h    \
		Image.h	   \
		PlyFile.h  \
		PointSet.h \
		Vec3D.hpp  \
		Vertex.h   \
		Camera.h
main_libray = \
		Color.cpp    \
		PointSet.cpp \
		Vertex.cpp   \
		Image.cpp    \
		PlyFile.c    \
		Camera.cpp 

main_SOURCES = \
		$(main_header) \
		$(main_libray) \
		main.cpp 

