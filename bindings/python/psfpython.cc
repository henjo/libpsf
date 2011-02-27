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

struct PSFScalar_to_python {
    static PyObject *convert(const PSFScalar *scalar) {
	if (const PSFDoubleScalar *p = dynamic_cast<const PSFDoubleScalar *>(scalar)) {
	    return PyFloat_FromDouble(p->value);
	} else if (const PSFInt32Scalar *p = dynamic_cast<const PSFInt32Scalar *>(scalar)) {
	    return PyInt_FromLong((int)*p);
	} else if (const PSFStringScalar *p = dynamic_cast<const PSFStringScalar *>(scalar)) {
	    return PyString_FromString(p->tostring().c_str());
	} else if(const StructScalar *p = dynamic_cast<const StructScalar *>(scalar)) {
	    return Struct_to_python::convert(p->value);
	} else {
	    throw NotImplemented();
	}
    }
};

struct PropertyMap_to_python {
    static PyObject *convert(const PropertyMap& propmap) {
	PyObject *dict = PyDict_New();

	for(PropertyMap::const_iterator i = propmap.begin(); i != propmap.end(); i++)
	    PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
			   PSFScalar_to_python::convert(i->second));	
	return dict;
    }
};

PyObject *Struct_to_python::convert(const Struct& s) {
    PyObject *dict = PyDict_New();
    
    for(Struct::const_iterator i = s.begin(); i != s.end(); i++) 
	PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
		       PSFScalar_to_python::convert(i->second));	
    
    return dict;
}


struct PSFVector_to_numpyarray {	
    static PyObject *convert(PSFVector *vec) {
	if (PSFDoubleVector *f64v = dynamic_cast<PSFDoubleVector *>(vec)) {
	    // Create numpy array
	    npy_intp dims[1] = { f64v->size() };
	    return PyArray_SimpleNewFromData(1, dims, PyArray_DOUBLE, &f64v->at(0));
	} else if (PSFComplexDoubleVector *cf64v = 
		   dynamic_cast<PSFComplexDoubleVector *>(vec)) {
	    // Create numpy array
	    npy_intp dims[1] = { cf64v->size() };
	    return PyArray_SimpleNewFromData(1, dims, PyArray_CDOUBLE, &cf64v->at(0));
	} else if (StructVector *sv = dynamic_cast<StructVector *>(vec)) {
	    // Create numpy array
	    npy_intp dims[1] = { sv->size() };
	    PyObject *a = PyArray_SimpleNew(1, dims, PyArray_OBJECT);

	    PyObject **ptr = (PyObject **) PyArray_DATA(a);	
	    for(unsigned int i=0; i < sv->size(); i++)
		ptr[i] = Struct_to_python::convert(sv->at(i));

	    // Delete source vector
	    delete(vec);
	    return a;
	} else
	    return NULL;
    }
};
    
void translate_exception(IncorrectChunk const& e) {
    std::stringstream msg; msg << "Incorrect chunk " << e.chunktype;
    PyErr_SetString(PyExc_RuntimeError, msg.str().c_str());
}

void test_exception() {
    throw(IncorrectChunk(1));
}

BOOST_PYTHON_MODULE(_psf)
{ 
    import_array();
    to_python_converter<PropertyMap, PropertyMap_to_python>();
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
	.def("get_signal_vector", &PSFDataSet::get_signal_vector,
	     return_value_policy<return_by_value>())
	.def("get_signal_scalar", &PSFDataSet::get_signal_scalar,
	     return_value_policy<return_by_value>())
	.def("get_header_properties", &PSFDataSet::get_header_properties,
	     return_value_policy<return_by_value>())
	.def("get_signal_properties", &PSFDataSet::get_signal_properties,
	     return_value_policy<return_by_value>())
    ;

    def("test_exception", test_exception);

    class_<IncorrectChunk> incorrectChunkClass("IncorrectChunk", init<int>());
    boost::python::register_exception_translator<IncorrectChunk>(&translate_exception);
}
