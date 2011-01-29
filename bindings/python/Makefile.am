pkglibdir = $(libdir)/@PYTHON_LIB@/site-packages/psf

pkglib_LTLIBRARIES= \
	_psf.la

_psf_la_SOURCES = \
	psfpython.cc

_psf_la_LIBADD = \
	../../src/libpsf.la
	${PYTHON_LIB} \
	${BOOST_PYTHON_LIB}

_psf_la_CXXFLAGS = \
	-I../../include \
	-I${PYTHON_INCLUDE_DIR}

_psf_la_LDFLAGS = \
	-module Python \
	-l${BOOST_PYTHON_LIB}
