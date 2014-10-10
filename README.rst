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

To build the python extension::

   ./autogen.sh --with-python
   make
   sudo make install


Running the tests
-----------------
Install cppunit, then compile and run the tests in the test dir::

    sudo apt-get install libcppunit-dev
    cd test
    make
    ./test_psfdataset
