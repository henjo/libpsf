import pylab

import libpsf

#d = libpsf.PSFDataSet("/nfs/home/henrik/pycircuit.repo/pycircuit/post/cds/test/psf/tran.tran") 
d = libpsf.PSFDataSet("../data/timeSweep")

pylab.plot(d.get_param_values(), d.get_signal_values('net21'))
pylab.show()
