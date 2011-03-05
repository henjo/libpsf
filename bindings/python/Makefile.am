pkgpython_PYTHON = __init__.py

pkgpyexec_LTLIBRARIES= \
	_psf.la

_psf_la_SOURCES = \
	psfpython.cc

_psf_la_LIBADD = \
	../../src/libpsf.la \
	${PYTHON_LIB} \
	${BOOST_PYTHON_LIB}

_psf_la_CXXFLAGS = \
	-I../../include \
	-I${PYTHON_INCLUDE_DIR} \
	-I${NUMPY_INCLUDE_DIR}

_psf_la_LDFLAGS = \
	-avoid-version -module \
	-l${BOOST_PYTHON_LIB}

_psf_la_DEPENDENCIES =\
	../../src/libpsf.la
