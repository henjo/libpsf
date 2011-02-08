#include "psf.h"
#include "psfdata.h"

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/class.hpp>
#include <boost/python/to_python_converter.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/return_value_policy.hpp>
#include <boost/python/dict.hpp>

#include <Python.h>
#include <numpy/arrayobject.h>

using namespace boost::python;

struct Struct_to_python {
    static PyObject *convert(const Struct& s);
};

struct PSFScalar_to_python {
    static PyObject *convert(PSFScalar *scalar) {
	if (PSFDoubleScalar *p = dynamic_cast<PSFDoubleScalar *>(scalar)) {
	    return PyFloat_FromDouble(p->value);
	} else if(StructScalar *p = dynamic_cast<StructScalar *>(scalar)) {
	    return Struct_to_python::convert(p->value);
	} else {
	    return NULL;
	}
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
    
BOOST_PYTHON_MODULE(_psf)
{ 
    import_array();
    to_python_converter<PSFVector *, PSFVector_to_numpyarray>();
    to_python_converter<PSFScalar *, PSFScalar_to_python>();
    to_python_converter<Struct, Struct_to_python>();

    //    class_<PSFVector>("PSFVector");

    class_< std::vector<std::string> >("StringVec")
	.def(vector_indexing_suite<std::vector<std::string> >())
    ;

    class_<PSFDataSet>("PSFDataSet", init<std::string>())
	.def("get_signal_names", &PSFDataSet::get_signal_names)
	.def("get_param_values", &PSFDataSet::get_param_values,
	     return_value_policy<return_by_value>())
	.def("get_signal_values", &PSFDataSet::get_signal_values,
	     return_value_policy<return_by_value>())
	.def("get_signal_value", &PSFDataSet::get_signal_value,
	     return_value_policy<return_by_value>())
    ;
}
