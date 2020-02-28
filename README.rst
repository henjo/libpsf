libpsf is a c++ library that reads Cadence PSF waveform files

Install
=======

Install prerequisits
--------------------

On a debian based system you can run the following to install the 
packages needed to build libpsf:

sudo apt-get install autoconf automake libtool libboost-all-dev python-numpy-dev 

Build and install
-----------------
To build and install the library::

   ./autogen.sh
   make
   sudo make install

To build the python extension with conda::
    
    conda install python=3.7 numpy automake libtool cython 

    # link python3.7 to python3.7m
    ln -s $CONDA_PREFIX/lib/libpython3.7m.so $CONDA_PREFIX/lib/libpython3.7.so
    
     
    ./autogen.sh 
    ./configure --prefix=$CONDA_PREFIX --with-python
    # make errors out with "cannot find the library 'libpsf.la'" so build libpsf.la first
    cd src
    make libpsf.la
    cd ..
    make install
    cd bindings/python
    python setup.py install


Running the tests
-----------------
Install cppunit, then compile and run the tests in the test dir::

    sudo apt-get install libcppunit-dev
    cd test
    make
    ./test_psfdataset
