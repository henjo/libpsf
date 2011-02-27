import libpsf

#d = libpsf.PSFDataSet("/nfs/home/henrik/pycircuit.repo/pycircuit/post/cds/test/psf/tran.tran") 
d = libpsf.PSFDataSet("../data/timeSweep")
print d.get_header_properties()


print list(d.get_signal_names())
t = d.get_sweep_values()
v = d.get_signal_vector('INP')

print len(v), len(t)

import pylab
pylab.plot(t, v)
pylab.show()

