#include "psf.h"
#include "psfinternal.h"
#include "psfdata.h"

PSFDataSet::PSFDataSet(std::string _filename) : filename(_filename), invertstruct(false) {
    psf = new PSFFile(filename.c_str());
    _open = false;
    psf->open();
}

PSFDataSet::~PSFDataSet() {
    if(_open)
	{
	    psf->close();
	    _open = false;
	}
    delete psf;
}

void PSFDataSet::close(){
    if (_open) {
	psf->close();
	_open = false;
    }
}

void PSFDataSet::open() {
    if (! _open) {
	psf->open();
	_open = true;
    }
}

const std::vector<std::string> PSFDataSet::get_signal_names() {
    open();
    return psf->get_names();
}

int PSFDataSet::get_nsweeps() {
    open();
    return *(psf->header->get_header_properties().find("PSF sweeps")->second);
}

int PSFDataSet::get_sweep_npoints() {
    open();
    return (int) *psf->header->get_property("PSF sweep points");
}

const std::vector<std::string> PSFDataSet::get_sweep_param_names() {
    open();
    return psf->get_param_names();
}

PSFVector *PSFDataSet::get_sweep_values() {	
    open();
    return psf->get_param_values();
}

PSFBase* PSFDataSet::get_signal(std::string name) {
    open();

    if (is_swept()) {
	PSFVector *vec = get_signal_vector(name);
	
	if(invertstruct && dynamic_cast<const StructVector *>(vec)) {
	    VectorStruct *vs = new VectorStruct(*dynamic_cast<const StructVector *>(vec));
	    
	    delete vec;
	    
	    return vs;
	} else
	    return vec;
    } else {
	// Convert to const
	PSFScalar *scalar = psf->get_value(name).clone();
	return scalar;
    }
}

PSFVector *PSFDataSet::get_signal_vector(std::string name) {	
    open();
    return psf->get_values(name);
}

const PSFScalar& PSFDataSet::get_signal_scalar(std::string name) {	
    open();
    return psf->get_value(name);
}

PropertyMap PSFDataSet::get_signal_properties(std::string name) {
    open();
    return psf->get_value_properties(name);
}

const PropertyMap& PSFDataSet::get_header_properties() {
    open();
    return psf->header->get_header_properties();
}

bool PSFDataSet::is_swept() {
    open();
    int nsweeps = *(psf->header->get_header_properties().find("PSF sweeps")->second);
    return (nsweeps > 0);
}
