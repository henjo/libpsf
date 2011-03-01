import libpsf

d = libpsf.PSFDataSet("../data/pss0.fd.pss")

print list(d.get_signal_names())

print d.get_sweep_values(), d.get_signal('1')
