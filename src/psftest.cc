#include "psf.h"
#include "psfdata.h"

#include <string>
#include <boost/foreach.hpp>

void noisesummary() {
    std::string pnoisefile("/nfs/home/henrik/spectre/1/pnoise.raw/pnoise_pout3g.pnoise");
    PSFDataSet psfnoise(pnoisefile);

    std::vector<std::string> names = psfnoise.get_signal_names();	

    double sum=0;
    for(std::vector<std::string>::iterator i=names.begin(); i != names.end(); i++) {
	if(*i != std::string("out")) {
	    StructVector *valvec = dynamic_cast<StructVector *>(psfnoise.get_signal_values(*i));
	    Struct& data = (*valvec)[3];
	    if(data.find(std::string("total")) != data.end())
		sum += (double)*data[std::string("total")];
	    delete(valvec);
	}
    }
    std::cout << "Total: " << sum << std::endl;
}

int main() {
    std::string dcopfile("../examples/data/opBegin");
    std::string pssfdfile("../examples/data/pss0.fd.pss");
    std::string tranfile("../examples/data/timeSweep");

    PSFDataSet psftran(tranfile);
    PSFDataSet pssfd(pssfdfile);
    PSFDataSet pssop(dcopfile);

    noisesummary();
    
    // std::cout << "Header properties:" << std::endl;
    // PropertyMap headerprops(psfnoise.get_header_properties());
    // for(PropertyMap::iterator i=headerprops.begin(); i!=headerprops.end(); i++) 
    // 	std::cout << i->first << ":" << *i->second << std::endl;    

    // {
    // 	Float64Vector *parvec = (Float64Vector *)psfnoise.get_param_values();
    // }


    
    // Float64Vector *parvec = (Float64Vector *)psftran.get_param_values();

    // for(Float64Vector::iterator i=parvec->begin(); i!=parvec->end(); i++)
    // 	std::cout << *i << " ";

    // std::cout << std::endl;

    // std::cout << "len=" << parvec->size() << std::endl;

    //PSFDataVector *valvec = psf.get_signal_values("tx_lopath_hb_stop.tx_lopath_hb_top.tx_lopath_hb_driver.driver_hb_channel_q.Ismall.Idriver_n.nout_off.imod");

}
