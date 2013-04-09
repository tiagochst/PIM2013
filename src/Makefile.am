# additional include paths necessary to compile the C++ programs
# Debug Flags
if DEBUG
AM_CPPFLAGS=-g -O0
AM_CXXFLAGS=-D__DEBUG_PLY_READ
else
AM_CFLAGS=-O3 -msse2
AM_CXXFLAGS=-O3 -msse2
AM_CPPFLAGS=-O3 -DNDEBUG
endif
AM_LDFLAGS=-lm -lglut

# Include flags
AM_CXXFLAGS+=$(GLU_CFLAGS) $(OPENNI_CFLAGS) $(GL_CFLAGS) 
AM_CPPFLAGS+=-Wno-write-strings -Wunused-result 
AM_CPPFLAGS+=-I$(srcdir)/../include -I$(srcdir)/../external
AM_LDFLAGS+=$(OPENNI_LIBS) $(GLU_LIBS) $(GL_LIBS) 

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

