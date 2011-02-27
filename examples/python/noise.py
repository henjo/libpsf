import libpsf

d = libpsf.PSFDataSet("/nfs/home/henrik/spectre/1/pnoise.raw/pnoise_pout3g.pnoise")

#print list(d.get_signal_names())

stotal = 0
for name in d.get_signal_names():
    if name != 'out':
        stotal += d.get_signal_vector(name)[3]['total']

print stotal
