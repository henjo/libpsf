import libpsf

d = libpsf.PSFDataSet("../data/frequencySweep")

print d.get_sweep_values()
print d.get_signal_vector('ANT_CM')
