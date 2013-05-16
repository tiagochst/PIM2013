# additional include paths necessary to compile the C++ programs
# Debug Flags
if DEBUG
AM_CFLAGS=-D__DEBUG_PLY_READ
AM_CXXFLAGS=-D__DEBUG_PLY_READ
AM_CPPFLAGS=-g -O0
else
AM_CFLAGS=
AM_CXXFLAGS=
AM_CPPFLAGS=-O3 -msse2 -DNDEBUG
endif

# Common optimisation flags
AM_CFLAGS+=-fopenmp
AM_CXXFLAGS+=-fopenmp

# Include flags
AM_CFLAGS+=-I$(top_builddir)/include -I$(top_builddir)/external
AM_CXXFLAGS+=-I$(top_builddir)/include -I$(top_builddir)/external

# External libraries CFLAGS
AM_CFLAGS+=$(OPENNI_CFLAGS) $(GLU_CFLAGS) $(GL_CFLAGS) 
AM_CXXFLAGS+=$(OPENNI_CFLAGS) $(GLU_CFLAGS) $(GL_CFLAGS) 

# External libraries
LDADD=$(OPENNI_LIBS)$(GLU_LIBS)$(GL_LIBS)-lm -lgomp -lglut

bin_PROGRAMS = $(top_builddir)/bin/main
__top_builddir__bin_main_SOURCES = \
		Color.cpp    \
		PointSet.cpp \
		Vertex.cpp   \
		Image.cpp    \
		PlyFile.c    \
		Camera.cpp   \
		Config.cpp   \
		main.cpp 

