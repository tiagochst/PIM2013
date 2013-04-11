# Prelude
AC_INIT([facialReconstruction],[1.0],[bug-reporte@adress])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PREREQ(2.69)

# Redefining compilers default flags
CPPFLAGS=
CXXFLAGS=
CFLAGS=
LDFLAGS=

AC_CONFIG_COMMANDS([mkdir], [$MKDIR_P bin bin/CapturedFrames resources/Project/Data/Output  resources/Project/Data/Output/CapturedFrames doc])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# Check for libraries 
PKG_CHECK_MODULES(OPENCV,opencv)
PKG_CHECK_MODULES(OPENNI,openni)
PKG_CHECK_MODULES(GLU,glu)
PKG_CHECK_MODULES(GLU,gl)


# Check for header files 
# Check for typedefs, structures, and compilers characteristics
# Check for library functions 

# Output Files
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])

# Debug condition
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_OUTPUT

