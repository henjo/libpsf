#include "psf.h"

#include <string>
#include <boost/foreach.hpp>


int main() {
    PSFFile psf("/nfs/home/henrik/spectre/1/pnoise.raw/pnoise_pout3g.pnoise");
    psf.open();

    // NameList names = psf.traces->get_names();
    // BOOST_FOREACH(std::string name, names) { 
    // 	std::cout << name << " "; 
    // }

    ChildList filter;
    filter.push_back(psf.traces->get_child(96226));
    
    SweepValueList result;
    
        result = ((ValueSectionSweep *)psf.values)->get_values(*psf.traces);    
    //result = ((ValueSectionSweep *)psf.values)->get_values(filter);    

    std::cout << result.size() << std::endl;

    Chunk *trace = psf.traces->get_child("tx_iqfilter_stop_0.tx_iqfilter_top_0.tx_iqfilter_bias_0.iprobe_gnd");

    std::cout << trace->get_name() << std::endl;

    psf.close();
}
