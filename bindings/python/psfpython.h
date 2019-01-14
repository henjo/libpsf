#include "Python.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "psfdata.h"

void raise_py_error();
PyObject* psfscalar_to_python(const PSFScalar *scalar);
PyObject* struct_to_python(const Struct& s); 
PyObject* create_numpy_vector(int n, int type, void *data, bool copy);
PyObject* psfvector_to_numpy_array(PSFVector *vec, bool copy=false);
PyObject* vector_struct_to_python(VectorStruct *vs);
PyObject* psfbase_to_numpy_array(PSFBase * d);
PyObject* propertymap_to_python(PropertyMap& propmap);
