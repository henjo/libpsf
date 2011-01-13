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

struct PSFData_to_python {
    static PyObject *convert(PSFData *value) {
	if (Float64 *p = dynamic_cast<Float64 *>(value)) {
	    return PyFloat_FromDouble(p->value);
	} else if(Struct *p = dynamic_cast<Struct *>(value)) {
	    PyObject *dict = PyDict_New();

	    for(Struct::iterator i = p->begin(); i != p->end(); i++)
		PyDict_SetItem(dict, PyString_FromString(i->first.c_str()), 
			       convert(i->second));

	    return dict;
	} else {
	    return NULL;
	}
    } 
};

struct PSFDataVector_to_numpyarray {	
    static PyObject *convert(PSFDataVector *vec) {
	if (Float64Vector *f64v = dynamic_cast<Float64Vector *>(vec)) {
	    // Create numpy array
	    npy_intp dims[1] = { f64v->size() };
	    PyObject *a = PyArray_SimpleNew(1, dims, PyArray_DOUBLE);

	    // Copy data to it
	    double *ptr = (double *) PyArray_DATA(a);	
	    for(unsigned int i=0; i < f64v->size(); i++)
		ptr[i] = (*f64v)[i].value;

	    // Delete source vector
	    delete(vec);

	    return a;
	} else if (StructVector *sv = dynamic_cast<StructVector *>(vec)) {
	    // Create numpy array
	    npy_intp dims[1] = { sv->size() };
	    PyObject *a = PyArray_SimpleNew(1, dims, PyArray_OBJECT);

	    PyObject **ptr = (PyObject **) PyArray_DATA(a);	
	    for(unsigned int i=0; i < sv->size(); i++)
		ptr[i] = PSFData_to_python::convert(&(*sv)[i]);

	    // Delete source vector
	    delete(vec);
	    return a;
	} else
	    return NULL;
    }
};
    
BOOST_PYTHON_MODULE(psf)
{ 
    import_array();
    to_python_converter<PSFDataVector *, PSFDataVector_to_numpyarray>();
    to_python_converter<PSFData *, PSFData_to_python>();

    class_<PSFDataVector>("PSFDataVector");

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
