
#CXXFLAGS = -O3 -fPIC #-g -pg
CXXFLAGS = -fPIC -g -pg

PSFLIB = libpsf.a

HEADERS = psf.h psfinternal.h psfdata.h

OBJS = psf.o psfdata.o psfproperty.o psfchunk.o psfcontainer.o psfindexedcontainer.o psfgroup.o \
	psffile.o psftype.o psfstruct.o psfsections.o psftrace.o psfnonsweepvalue.o psfsweepvalue.o

all: psftest $(PSFLIB) psf.so

$(PSFLIB): $(OBJS)
	$(AR) rc $@ $(OBJS)

psfreader.o: $(HEADERS) psfreader.cc
psfdata.o: $(HEADERS) psfdata.cc
psf.o: $(HEADERS) psf.cc

psftest: psftest.o $(OBJS) psf.h
	g++ $(CXXFLAGS) $(OBJS) -o $@ $<

clean:
	rm -fr psftest.o $(OBJS) $(PSFLIB)

psf.so:	$(PSFLIB) setup.py psfpython.cc
	rm -fr build
	python setup.py build
	cp build/lib.linux-x86_64-2.6/psf.so .
