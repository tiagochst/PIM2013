# additional include paths necessary to compile the C++ programs

# Debug Flags
if DEBUG
AM_CPPFLAGS=-g -O0
AM_CXXFLAGS=-D__DEBUG_PLY_FILE
else
AM_CFLAGS = -O3
AM_CXXFLAGS =-O3
AM_CPPFLAGS =-O3
endif

# Include flags
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
		Vertex.h    
main_libray = \
		Color.cpp    \
		PointSet.cpp \
		Vertex.cpp   \
		Image.cpp    \
		PlyFile.c

main_SOURCES = \
		$(main_header) \
		$(main_libray) \
		main.cpp 

