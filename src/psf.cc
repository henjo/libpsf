#include "psf.h"
#include "psfinternal.h"
#include "psfdata.h"

PSFDataSet::PSFDataSet(std::string filename) : m_filename(filename), m_invertstruct(false) {
    m_psf     = new PSFFile(m_filename.c_str());
    m_is_open = false;
    
    open();
}

PSFDataSet::~PSFDataSet() {
    m_psf->close();
    delete m_psf;
}

void PSFDataSet::close() {
    if (m_is_open) {
	m_psf->close();
	m_is_open = false;
    }
}

void PSFDataSet::open() {
    if (! m_is_open) {
	m_psf->open();
	m_is_open = true;
    }
}

const std::vector<std::string> PSFDataSet::get_signal_names() const {
    verify_open();

    return m_psf->get_names();
}

int PSFDataSet::get_nsweeps() const {
    verify_open();

    return (int) m_psf->get_header_properties().find("PSF sweeps");
}

int PSFDataSet::get_sweep_npoints() const {
    verify_open();

    return (int) m_psf->get_header_properties().find("PSF sweep points");
}

const std::vector<std::string> PSFDataSet::get_sweep_param_names() const {
    verify_open();

    return m_psf->get_param_names();
}

PSFVector *PSFDataSet::get_sweep_values() const {	
    verify_open();

    return m_psf->get_param_values();
}

PSFBase* PSFDataSet::get_signal(std::string name) const {
    verify_open();

    if (is_swept()) {
	PSFVector *vec = get_signal_vector(name);
	
	if(m_invertstruct && dynamic_cast<const StructVector *>(vec)) {
	    VectorStruct *vs = new VectorStruct(*dynamic_cast<const StructVector *>(vec));
	    
	    delete vec;
	    
	    return vs;
	} else
	    return vec;
    } else {
	// Convert to const
	PSFScalar *scalar = m_psf->get_value(name).clone();
	return scalar;
    }
}

PSFVector *PSFDataSet::get_signal_vector(std::string name) const {	
    verify_open();

    return m_psf->get_values(name);
}

const PSFScalar& PSFDataSet::get_signal_scalar(std::string name) const {	
    verify_open();

    return m_psf->get_value(name);
}

const PropertyMap & PSFDataSet::get_signal_properties(std::string name) const {
    verify_open();

    if (is_swept()){
        throw NotFound();
    }

    return m_psf->get_value_properties(name).get_propmap();
}

const PropertyMap & PSFDataSet::get_header_properties() const {
    verify_open();

    return m_psf->get_header_properties().get_propmap();
}

bool PSFDataSet::is_swept() const {
    verify_open();

    const int nsweeps = m_psf->get_header_properties().find("PSF sweeps");

    return nsweeps > 0;
}

void PSFDataSet::set_invertstruct(bool value) {
    verify_open();
    m_invertstruct = value; 
}
 
bool PSFDataSet::get_invertstruct() const {
    verify_open();
    return m_invertstruct;  
}

inline void PSFDataSet::verify_open() const {
    if (!m_is_open) {
	std::cerr << "Data set is not open" << std::endl;
	throw DataSetNotOpen();
    }
}
