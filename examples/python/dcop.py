import libpsf

d = libpsf.PSFDataSet("../data/opBegin")

#print list(d.get_signal_names())

print d.get_signal_scalar('XIRXRFMIXTRIM0.XM1PDAC6.XMN.MAIN')
