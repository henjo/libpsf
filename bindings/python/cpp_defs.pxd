# https://cython.readthedocs.io/en/latest/src/userguide/wrapping_CPlusPlus.html
# https://stackoverflow.com/a/10121232
# https://stackoverflow.com/a/29002414
from cpython.ref cimport PyObject

from libcpp cimport bool
from libcpp.string cimport string
from libcpp.vector cimport vector

cdef extern from "psf.h":
    cdef cppclass PSFVector
    cdef cppclass PSFBase

cdef extern from "psfpython.h":
    cdef void raise_py_error()
    cdef cppclass PropertyMap

cdef extern from "psf.h":
    cdef cppclass PSFDataSet:
        PSFDataSet(string) except +raise_py_error
        int get_nsweeps()  except +raise_py_error
        int get_sweep_npoints() except +raise_py_error
        const vector[string] get_signal_names() except +raise_py_error
        const vector[string] get_sweep_param_names() except +raise_py_error
        PSFVector* get_sweep_values() except +raise_py_error
        PSFBase *get_signal(string name) except +raise_py_error
        PropertyMap& get_signal_properties(string name) except +raise_py_error
        PropertyMap& get_header_properties() except +raise_py_error
        bool is_swept() except +raise_py_error
        void close() except +raise_py_error
        void open() except +raise_py_error
        void set_invertstruct(bool value) except +raise_py_error
        bool get_invertstruct() except +raise_py_error
