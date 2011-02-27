import libpsf

d = libpsf.PSFDataSet("../data/srcSweep")

print list(d.get_signal_names())

print d.get_signal_vector('VIN')
