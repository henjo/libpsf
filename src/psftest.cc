#include "psf.h"
#include "psfdata.h"

#include <string>
#include <boost/foreach.hpp>

struct apa {
    uint32_t a;
};

int main() {
    std::string pnoisefile("/nfs/home/henrik/spectre/1/pnoise.raw/pnoise_pout3g.pnoise");
    std::string dcopfile("/nfs/home/henrik/spectre/1/dc.raw/dcOpInfo.info");
    std::string tranfile("/nfs/home/henrik/pycircuit.repo/pycircuit/post/cds/test/psf/tran.tran");

    PSFDataSet psftran(tranfile);

    //    psftran.get_signal_names();
    Float64Vector *parvec = (Float64Vector *)psftran.get_param_values();

    for(Float64Vector::iterator i=parvec->begin(); i!=parvec->end(); i++)
	std::cout << *i << " ";

    std::cout << std::endl;

    std::cout << "len=" << parvec->size() << std::endl;

    //PSFDataVector *valvec = psf.get_signal_values("tx_lopath_hb_stop.tx_lopath_hb_top.tx_lopath_hb_driver.driver_hb_channel_q.Ismall.Idriver_n.nout_off.imod");

}
