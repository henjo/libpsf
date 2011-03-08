#include "psf.h"
#include "psfdata.h"

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/class.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/exception_translator.hpp>
#include <boost/python/dict.hpp>

#include <sstream>

#include <Python.h>
#include <numpy/arrayobject.h>

using namespace boost::python;

struct Struct_to_python {
    static PyObject *convert(const Struct& s);
};

PyObject *psfscalar_to_python(const PSFScalar *scalar) {
    PyObject *result = NULL;
    if (const PSFDoubleScalar *p = dynamic_cast<const PSFDoubleScalar *>(scalar))
	result = PyFloat_FromDouble(p->value);
    else if (const PSFInt32Scalar *p = dynamic_cast<const PSFInt32Scalar *>(scalar))
	result = PyInt_FromLong((int)*p);
    else if (const PSFStringScalar *p = dynamic_cast<const PSFStringScalar *>(scalar))
	result = PyString_FromString(p->tostring().c_str());
    else if(const StructScalar *p = dynamic_cast<const StructScalar *>(scalar))
	result = Struct_to_python::convert(p->value);
    else
	throw NotImplemented();
    
    return result;
}

struct PSFScalar_to_python {
    static PyObject *convert(const PSFScalar *scalar) {
	PyObject *result = psfscalar_to_python(scalar);
	delete scalar;	
	return result;
    }
};

struct PropertyMap_to_python {
    static PyObject *convert(const PropertyMap& propmap) {
	PyObject *dict = PyDict_New();

	for(PropertyMap::const_iterator i = propmap.begin(); i != propmap.end(); i++)
	    PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
			   psfscalar_to_python(i->second));	
	return dict;
    }
};

PyObject *Struct_to_python::convert(const Struct& s) {
    PyObject *dict = PyDict_New();
    
    for(Struct::const_iterator i = s.begin(); i != s.end(); i++) {
	PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
		       psfscalar_to_python(i->second));
    }	
    
    return dict;
}

PyObject *create_numpy_vector(int n, int type, void *data, bool copy) {
    npy_intp dims[1] = { n };

    if(copy) {
	PyObject *result = PyArray_SimpleNew(1, dims, type);
	void *arr_data = PyArray_DATA((PyArrayObject*)result);
	memcpy(arr_data, data, PyArray_ITEMSIZE((PyArrayObject*) result) * n);
	return result;
    } else 
	return PyArray_SimpleNewFromData(1, dims, type, data);
}

PyObject *psfvector_to_numpyarray(PSFVector *vec, bool copy=false) {
    PyObject *result = NULL;

    if (PSFDoubleVector *f64v = dynamic_cast<PSFDoubleVector *>(vec)) {
	// Create numpy array
	result = create_numpy_vector(f64v->size(), PyArray_DOUBLE, &f64v->at(0), copy);
    } else if (PSFComplexDoubleVector *cf64v = 
	       dynamic_cast<PSFComplexDoubleVector *>(vec)) {
	// Create numpy array
	result = create_numpy_vector(cf64v->size(), PyArray_CDOUBLE, &cf64v->at(0), copy);
    } else if (StructVector *sv = dynamic_cast<StructVector *>(vec)) {
	// Create numpy array
	npy_intp dims[1] = { sv->size() };
	result = PyArray_SimpleNew(1, dims, PyArray_OBJECT);
	    
	PyObject **ptr = (PyObject **) PyArray_DATA(result);
	for(unsigned int i=0; i < sv->size(); i++)
	    ptr[i] = Struct_to_python::convert(sv->at(i));

	// Make source vector is deleted
	copy = true;
    }
    if(copy)
	delete vec;
    
    return result;
}

PyObject *vectorstruct_to_python(VectorStruct *vs) {
    // Create dictionary of numpy arrays
    PyObject *dict = PyDict_New();
		
    for(VectorStruct::const_iterator i = vs->begin(); i != vs->end(); i++)
	PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
		       psfvector_to_numpyarray(i->second, true));	
    return dict;
}

struct PSFVector_to_numpyarray {	
    static PyObject *convert(PSFVector *vec) {
	return psfvector_to_numpyarray(vec);
    }
};

struct PSFBase_to_numpyarray {	
    static PyObject *convert(PSFBase *d) {
	const PSFScalar *scalar = dynamic_cast<const PSFScalar *>(d);
	if (scalar != NULL)
	    return psfscalar_to_python(scalar);
	else {	
	    PSFVector *vector = dynamic_cast<PSFVector *>(d);
	    if (vector != NULL)
		return psfvector_to_numpyarray(vector, true);	
	    else {
		VectorStruct *vs = dynamic_cast<VectorStruct *>(d);

		if(vs != NULL)
		    return vectorstruct_to_python(vs);
	    }
	}
    }
};

// Exception translators    
void translate_exception(IncorrectChunk const& e) {
    std::stringstream msg; msg << "Incorrect chunk " << e.chunktype;
    PyErr_SetString(PyExc_RuntimeError, msg.str().c_str());
}

void translate_exception_unknown_type(UnknownType const& e) {
    std::stringstream msg; msg << "Unknown type " << e.type_id;
    PyErr_SetString(PyExc_RuntimeError, msg.str().c_str());
}

void translate_exception_notfound(NotFound const& e) {
    std::stringstream msg; msg << "Signal not found";
    PyErr_SetString(PyExc_RuntimeError, msg.str().c_str());
}

void translate_exception_fileopenerror(FileOpenError const& e) {
    std::stringstream msg; msg << "File open error";
    PyErr_SetString(PyExc_IOError, msg.str().c_str());
}


BOOST_PYTHON_MODULE(_psf)
{ 
    import_array();
    to_python_converter<PropertyMap, PropertyMap_to_python>();
    to_python_converter<PSFBase *, PSFBase_to_numpyarray>();
    to_python_converter<PSFVector *, PSFVector_to_numpyarray>();
    to_python_converter<const PSFScalar *, PSFScalar_to_python>();
    to_python_converter<Struct, Struct_to_python>();

    class_< std::vector<std::string> >("StringVec")
	.def(vector_indexing_suite<std::vector<std::string> >())
    ;

    class_<PSFDataSet>("PSFDataSet", init<std::string>())
	.def("get_sweep_npoints", &PSFDataSet::get_sweep_npoints)
	.def("get_signal_names", &PSFDataSet::get_signal_names)
	.def("get_sweep_values", &PSFDataSet::get_sweep_values,
	     return_value_policy<return_by_value>())
	.def("get_signal", &PSFDataSet::get_signal, 
	     return_value_policy<return_by_value>())
	.def("get_header_properties", &PSFDataSet::get_header_properties,
	     return_value_policy<return_by_value>())
	.def("get_signal_properties", &PSFDataSet::get_signal_properties,
	     return_value_policy<return_by_value>())
	.def("is_swept", &PSFDataSet::is_swept)
	.add_property("invertstruct",
		      &PSFDataSet::get_invertstruct,
		      &PSFDataSet::set_invertstruct)
    ;

    class_<IncorrectChunk> incorrectChunkClass("IncorrectChunk", init<int>());
    //    class_<NotFound> incorrectChunkClass("NotFound", init<>());
    boost::python::register_exception_translator<IncorrectChunk>(&translate_exception);
    boost::python::register_exception_translator<NotFound>(&translate_exception_notfound);
    boost::python::register_exception_translator<FileOpenError>(&translate_exception_fileopenerror);
    boost::python::register_exception_translator<UnknownType>(&translate_exception_unknown_type);
}
