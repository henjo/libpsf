lib_LTLIBRARIES = libpsf.la

bin_PROGRAMS = psftest
psftest_SOURCES = psftest.cc
psftest_CXXFLAGS = -I../include ${BOOST_CPPFLAGS}
psftest_LDFLAGS = libpsf.la

libpsf_la_SOURCES = psf.cc psfdata.cc psfproperty.cc psfchunk.cc \
	psfcontainer.cc psfindexedcontainer.cc psfgroup.cc psffile.cc \
	psftype.cc psfstruct.cc psfsections.cc psftrace.cc \
	psfnonsweepvalue.cc psfsweepvalue.cc psfpropertyblock.cc

libpsf_la_CXXFLAGS = \
	-I../include ${BOOST_CPPFLAGS}


