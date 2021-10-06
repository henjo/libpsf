// patches from https://gitlab.com/bjmuld/libpsf-python

#include <exception>
#include <string>

#include "Python.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "psf.h"
#include "psfdata.h"
#include "psfpython.h"

#ifndef PY_MAJOR_VERSION
    #error could not find python version from python headers
#else
    #if PY_MAJOR_VERSION >= 3
        #warning "detected python 3.x";
	#define STRING_INPUT_FN PyUnicode_FromString
	#define LONG_INPUT_FN PyLong_FromLong
    #else
        #warning "detected python 2.x";
	#define STRING_INPUT_FN PyString_FromString
	#define LONG_INPUT_FN PyInt_FromLong
    #endif
#endif

// init_numpy hack https://stackoverflow.com/questions/47026900/pyarray-check-gives-segmentation-fault-with-cython-c
int init_numpy(){
    if(_import_array() < 0){
        PyErr_Print();
        PyErr_SetString(PyExc_ImportError, "numpy.core.multiarray failed to import");
        return false;
    }
    return true;
}

const static int numpy_initialized = init_numpy();

extern PyObject *fileOpenError;
extern PyObject *notFoundError;
extern PyObject *notOpenError;
extern PyObject *unknownTypeError;
extern PyObject *incorrectChunkError;

void raise_py_error(){
    try{
        throw;
    }catch (FileOpenError& e){
        std::string msg = "File Open Error";
        PyErr_SetString(fileOpenError, msg.c_str());
    }catch (NotFound& e){
        std::string msg = "Signal not found";
        PyErr_SetString(notFoundError, msg.c_str());
    }catch(DataSetNotOpen& e){
        std::string msg = "Dataset not open";
        PyErr_SetString(notOpenError, msg.c_str());
    }catch(UnknownType& e){
        std::stringstream msg; msg << "Unknown type " << e.type_id;
        PyErr_SetString(unknownTypeError, msg.str().c_str());
    }catch (IncorrectChunk& e){
        std::stringstream msg; msg << "Incorrect chunk " << e.chunktype;
        PyErr_SetString(incorrectChunkError, msg.str().c_str());
    }catch (const std::exception & e){
        PyErr_SetString(PyExc_RuntimeError, "general exception");
    }
}

PyObject *psfscalar_to_python(const PSFScalar *scalar) {
    PyObject *result = NULL;
    if (const PSFDoubleScalar *p = dynamic_cast<const PSFDoubleScalar *>(scalar))
        result = PyFloat_FromDouble(p->value);
    else if (const PSFInt32Scalar *p = dynamic_cast<const PSFInt32Scalar *>(scalar))
        result = LONG_INPUT_FN((int)*p);
    else if (const PSFStringScalar *p = dynamic_cast<const PSFStringScalar *>(scalar))
        result = STRING_INPUT_FN(p->tostring().c_str());
    else if(const StructScalar *p = dynamic_cast<const StructScalar *>(scalar))
        result = struct_to_python(p->value);
    else
        throw NotImplemented();

    return result;
}

PyObject* struct_to_python(const Struct& s){
    PyObject *dict = PyDict_New();

    for(Struct::const_iterator i = s.begin(); i != s.end(); i++) {
        PyDict_SetItem(dict, STRING_INPUT_FN(i->first.c_str()), 
                psfscalar_to_python(i->second));
    }	

    return dict;

}

PyObject* vector_struct_to_python(VectorStruct *vs) {
    // Create dictionary of numpy arrays
    PyObject *dict = PyDict_New();

    for(VectorStruct::const_iterator i = vs->begin(); i != vs->end(); i++)
        PyDict_SetItem(dict, STRING_INPUT_FN(i->first.c_str()), 
                (PyObject *) psfvector_to_numpy_array(i->second, true));	
    return dict;
}

PyObject* create_numpy_vector(int n, int type, void *data, bool copy) {
    npy_intp dims[1] = { n };

    if(copy) {
        PyObject *result = PyArray_SimpleNew(1, dims, type);
        void *arr_data = PyArray_DATA((PyArrayObject*)result);
        memcpy(arr_data, data, PyArray_ITEMSIZE((PyArrayObject*) result) * n);
        return result;
    } else 
        return PyArray_SimpleNewFromData(1, dims, type, data);
}

PyObject* psfvector_to_numpy_array(PSFVector *vec, bool copy) {
    PyObject *result = NULL;

    if (PSFDoubleVector *f64v = dynamic_cast<PSFDoubleVector *>(vec)) {
        // Create numpy array
        result = create_numpy_vector(f64v->size(), NPY_DOUBLE, &f64v->at(0), copy);
    } else if (PSFComplexDoubleVector *cf64v = 
            dynamic_cast<PSFComplexDoubleVector *>(vec)) {
        // Create numpy array
        result = create_numpy_vector(cf64v->size(), NPY_CDOUBLE, &cf64v->at(0), copy);
    } else if (StructVector *sv = dynamic_cast<StructVector *>(vec)) {
        // Create numpy array
        npy_intp dims[1] = { (int) sv->size() };
        result = PyArray_SimpleNew(1, dims, NPY_OBJECT);

        PyObject **ptr = (PyObject **) PyArray_DATA((PyArrayObject*) result);
        for(unsigned int i=0; i < sv->size(); i++)
            ptr[i] = struct_to_python(sv->at(i));

        // Make source vector is deleted
        copy = true;
    } else if (vec == NULL) {
        result = Py_None;
    }

    if(copy && (vec != NULL))
        delete vec;

    return result;
}

PyObject* psfbase_to_numpy_array(PSFBase * d){
    const PSFScalar *scalar = dynamic_cast<const PSFScalar *>(d);
    if (scalar != NULL)
        return psfscalar_to_python(scalar);
    else {	
        PSFVector *vector = dynamic_cast<PSFVector *>(d);
        if (vector != NULL)
            return psfvector_to_numpy_array(vector, true);	
        else {
            VectorStruct *vs = dynamic_cast<VectorStruct *>(d);

            if(vs != NULL)
                return vector_struct_to_python(vs);
            else
                return Py_None;
        }
    }
}

PyObject* propertymap_to_python(PropertyMap& propmap){
    PyObject *dict = PyDict_New();

    for(PropertyMap::const_iterator i = propmap.begin(); i != propmap.end(); i++)
        PyDict_SetItem(dict, STRING_INPUT_FN(i->first.c_str()), 
                psfscalar_to_python(i->second));	
    return dict;

}

