#!/usr/bin/env python

import os, sys
import numpy
from sysconfig import get_paths

# BEFORE importing setuptools, remove MANIFEST. Otherwise it may not be
# properly updated when the contents of directories change (true for distutils,
# not sure about setuptools).
if os.path.exists('MANIFEST'):
  os.remove('MANIFEST')

from setuptools import setup, Extension
from Cython.Build import cythonize

try:
    with open('README.md', "r") as fh:
        long_description = fh.read()
except:
    long_description = ''

root_include = os.path.abspath(os.path.join(get_paths()['include'], '..'))

libpsf_ext = Extension(
    name="libpsf",
    sources = ["libpsf.pyx", "psfpython.cc"],
    libraries = ["psf"],
    include_dirs = [root_include, numpy.get_include() ],
)

setup(
    name="libpsf",
    ext_modules=cythonize([libpsf_ext]),
    version="0.0.1",
    description="library to read Cadence PSF output",
    install_requires=['numpy>=1.10.0'],
    test_suite="tests",
    # metadata to display on PyPI
    author="@lekez2005 originally Henrik Johansen",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="GNU Lesser General Public License v3.0",
    keywords=["cadence","spectre","virtuoso","circtuit", "simulation",
              "waveform", "circuit simulation"],
    zip_safe=False
)
