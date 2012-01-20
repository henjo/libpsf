#!/usr/bin/env python
 
from setuptools import setup
from setuptools.extension import Extension
import os, sys, platform
from distutils.command.build_ext import build_ext
from setuptools import Command


libpsf_ext = Extension(name = "libpsf", 
                       sources = ["psfpython.cc"],
                       libraries = ["boost_python", "psf"])

import numpy
libpsf_ext.include_dirs.append(numpy.get_include())

setup(
    name="libpsf",
    ext_modules=[libpsf_ext],
    package_dir = {"" : "."},
    packages=["tests"],
    #tests_require=["mock"],
    test_suite="tests",
    )
