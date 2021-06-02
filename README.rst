libpsf is a c++ library that reads Cadence PSF waveform files

Install
=======

Install prerequisites
--------------------
If building without python binding, only cmake and boost are required

- On a debian based system you can run the following to install the 
packages needed to build libpsf:

    $ sudo apt-get install cmake libboost-all-dev python-numpy-dev cython cppunit

- Otherwise conda can be used to install the following packages:

    $ conda install python numpy cython cmake
    
    Then install boost libraries and set
    
    $ export BOOST_LOC=<BOOST_LOCATION>

Build and install
-----------------
- From root directory, create build directory

    $ mkdir build && cd build
- Run cmake configuration

    $ cmake .. -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DCMAKE_BUILD_TYPE=RELEASE -DWITH_PYTHON=ON

    `CONDA_PREFIX` is the destination where you want libpsf to be installed
    To build without the python binding, just set `-DWITH_PYTHON=OFF`
- Build

    $ make
- To run tests, [cppunit](https://www.freedesktop.org/wiki/Software/cppunit) is required.
    
    $ ctest

    `ctest --verbose` to see individual test result outputs

- Install

    $ make install
 
