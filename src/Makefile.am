# additional include paths necessary to compile the C++ programs
AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../external

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
		Image.cpp

main_SOURCES = \
		$(main_header) \
		$(main_libray) \
		main.cpp 

