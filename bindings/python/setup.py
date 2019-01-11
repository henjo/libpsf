#!/usr/bin/env python

import os, sys
import numpy
from sysconfig import get_paths

# BEFORE importing setuptools, remove MANIFEST. Otherwise it may not be
# properly updated when the contents of directories change (true for distutils,
# not sure about setuptools).
if os.path.exists('MANIFEST'):
  os.remove('MANIFEST')

from setuptools import setup, Extension, find_packages
#from distutils.core import setup, Extension


pystr = str(sys.version_info[0]) + str(sys.version_info[1])
boostlib = "boost_python" + pystr

if '27mu' in sys.executable:
    boostlib += '-u'




#print("############################################################")
#print("##   sysex: {}                       ".format(sys.executable))
#print("##   looking for a library named:  {}      ".format(boostlib))
#print("##   current dir:  {}                     ".format( os.getcwd() ) )
#print("##   dir contents:  {}                     ".format( os.listdir('.')))
#print("##   parent dir contents:  {}                     ".format( os.listdir('..')))
#print("############################################################")

try:
    with open('README.md', "r") as fh:
        long_description = fh.read()
except:
    long_description = ''

sysinclude = os.path.abspath(os.path.join(get_paths()['include'], '..'))
print(sysinclude)

libpsf_ext = Extension(
  name = "libpsf",
  sources = ["psfpython.cc"],
  libraries = [boostlib, "psf"],
  runtime_library_dirs = ['./lib'],
  library_dirs = ['./lib'],
  include_dirs = [sysinclude, numpy.get_include() ],
)

setup(
    name="libpsf",
    ext_modules=[libpsf_ext],
    version="0.1.2",
    description="library to read Cadence PSF output",
    install_requires=['numpy>=1.10.0'],
    packages=find_packages(),
    test_suite="tests",
    tests_require=["mock"],
    # metadata to display on PyPI
    author="Barry Muldrey, originally Henrik Johansen",
    author_email="barry@muldrey.net",
    long_description=long_description,
    long_description_content_type="text/markdown",
    license="GNU Lesser General Public License v3.0",
    keywords=["cadence","spectre","virtuoso","circtuit", "simulation", "waveform", "circuit simulation"],
    url="https://gitlab.com/bjmuld/libpsf-python/wikis/home",   # project home page, if any
    project_urls={
        "Bug Tracker": "https://gitlab.com/bjmuld/libpsf-python/issues",
        "Documentation": "https://gitlab.com/bjmuld/libpsf-python/wikis/home",
        "Source Code": "https://gitlab.com/bjmuld/libpsf-python",
    }
)
