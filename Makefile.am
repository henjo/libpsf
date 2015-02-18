ACLOCAL_AMFLAGS = -I m4

if ENABLE_TESTS
  MAYBE_TEST = test
endif

SUBDIRS = \
	src \
	include \
	bindings \
	$(MAYBE_TEST)

pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libpsf.pc

README: README.rst
