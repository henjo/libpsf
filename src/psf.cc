#include "psf.h"
#include "psfinternal.h"
#include "psfdata.h"

PSFDataSet::PSFDataSet(std::string _filename) : filename(_filename) {
    psf = new PSFFile(filename.c_str());
    psf->open();
}

PSFDataSet::~PSFDataSet() {
    psf->close();
    delete psf;
}

std::vector<std::string> PSFDataSet::get_signal_names() {
    return psf->get_names();
}

PSFDataVector *PSFDataSet::get_param_values() {	
    return psf->get_param_values();
}

PSFDataVector *PSFDataSet::get_signal_values(std::string name) {	
    return psf->get_values(name);
}

PSFData *PSFDataSet::get_signal_value(std::string name) {	
    return psf->get_value(name);
}
