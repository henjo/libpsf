import libpsf

d1 = libpsf.PSFDataSet("../data/opBegin")
d2 = libpsf.PSFDataSet("../data/dcOpInfo.info")

#print list(d.get_signal_names())

print d1.get_signal('XIRXRFMIXTRIM0.XM1PDAC6.XMN.MAIN')
print d2.get_signal('DACTOP_0.DIV2_1.MN27.mm4ynj')
