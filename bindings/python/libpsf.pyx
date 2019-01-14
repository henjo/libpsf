# distutils: language = c++
# cython: language_level=2

from libcpp cimport bool

from cpython.ref cimport PyObject
from cpp_defs cimport PSFVector, PSFBase, PropertyMap
from cpp_defs cimport PSFDataSet as C_PSFDataSet

cimport numpy as np
np.import_array()

cdef extern from "psfpython.h":
    cdef np.ndarray psfvector_to_numpy_array(PSFVector *vec, bool copy)
    cdef object psfbase_to_numpy_array(PSFBase *d)
    cdef object propertymap_to_python(PropertyMap& propmap)

class FileOpenError(RuntimeError):
    pass

class NotFound(RuntimeError):
    pass

class DataSetNotOpen(RuntimeError):
    pass

class UnknownType(RuntimeError):
    pass

class IncorrectChunk(RuntimeError):
    pass

cdef public PyObject* fileOpenError = <PyObject*>FileOpenError
cdef public PyObject* notFoundError = <PyObject*>NotFound
cdef public PyObject* notOpenError = <PyObject*>DataSetNotOpen
cdef public PyObject* unknownTypeError = <PyObject*>UnknownType
cdef public PyObject* incorrectChunkError = <PyObject*>IncorrectChunk


cdef to_unicode(str_):
    return str_.encode('UTF-8')

cdef bytes_to_string(str_list):
    return map(lambda x: x.decode('UTF-8'), str_list)


cdef class PSFDataSet:
    cdef C_PSFDataSet *obj

    def __cinit__(self, filename):
        self.obj = new C_PSFDataSet(to_unicode(filename))

    def __dealloc__(self):
        del self.obj

    def get_nsweeps(self):
        """Return the number of sweeps"""
        return self.obj.get_nsweeps()

    def get_sweep_npoints(self):
        """Return the number of points in the sweep"""
        return self.obj.get_sweep_npoints()

    def get_signal_names(self):
        """Return a list of signal names"""
        return bytes_to_string(self.obj.get_signal_names())

    def get_sweep_param_names(self):
        """Parameters that have been swept"""
        return bytes_to_string(self.obj.get_sweep_param_names())

    def get_sweep_values(self):
        """Numpy array of swept values"""
        return psfvector_to_numpy_array(self.obj.get_sweep_values(), False)

    def get_signal(self, signal):
        """Numpy array of signal values"""
        return psfbase_to_numpy_array(self.obj.get_signal(to_unicode(signal)))

    def get_signal_properties(self, signal):
        """Properties of a non swept signal
        Throws NotFound exception for non_swept datasets so check self.is_swept
        first
        """
        return propertymap_to_python(self.obj.get_signal_properties(to_unicode(signal)))

    def get_header_properties(self):
        """Dict of header [rp[ertoes and values"""
        return propertymap_to_python(self.obj.get_header_properties())

    def is_swept(self):
        """Is the data swept"""
        return self.obj.is_swept()

    def close(self):
        """Close PSF Data Set"""
        self.obj.close()

    def open(self):
        """Open PSF Data Set"""
        self.obj.open()

    def set_invertstruct(self, bool value):
        """Set invert struct"""
        self.obj.set_invertstruct(value)

    def get_invertstruct(self):
        """Get invert struct"""
        return self.obj.get_invertstruct()

