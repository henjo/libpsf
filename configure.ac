#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([libpsf],[0.0],[henrik@johome.net])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10 -Wall no-define])

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_LIBTOOL

dnl Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h stdint.h stdlib.h string.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

dnl Checks for python
AX_PYTHON
AX_PYTHON_DEVEL

dnl Checks for boost 
AX_BOOST_BASE([1.33.0])

dnl check to build python bindings
AX_BOOST_PYTHON

dnl Checks for boost python and numpy
AM_PATH_PYTHON(2.5, have_python=true, have_python=false)

dnl Check for numpy
AC_PYTHON_MODULE(numpy, fatal)

CPPFLAGS_SAVE=$CPPFLAGS
if test x$PYTHON_INCLUDE_DIR != x; then
    CPPFLAGS=-I$PYTHON_INCLUDE_DIR $CPPFLAGS
fi
NUMPY_DIR="${PYTHON_SITE_PKG}/numpy"
AC_ARG_WITH([numpy], [AS_HELP_STRING([--with-numpy=DIR],[directory where numpy is installed
                      [default=${PYTHON_SITE_PKG}/numpy] ])],
        NUMPY_DIR=$withval)
[NUMPY_DIR="${NUMPY_DIR}/core/include/numpy"]
AC_CHECK_HEADER([${NUMPY_DIR}/arrayobject.h],
                [NUMPY_HEADER=yes],
                [AC_MSG_ERROR([Numpy extension header not found])],
                [#include "Python.h"])
AC_SUBST(NUMPY_HEADER)
CPPFLAGS_SAVE=$CPPFLAGS

dnl Checks for library functions.
AC_FUNC_MMAP

dnl Include libtool to build shared libraries
dnl LT_INIT

AC_CONFIG_FILES([Makefile
                 src/Makefile
		 bindings/Makefile
		 bindings/python/Makefile
		 libpsf.pc
		 libpsf-uninstalled.pc
		 ])
AC_OUTPUT

