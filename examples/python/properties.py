import libpsf

import libpsf

d = libpsf.PSFDataSet("../data/opBegin")

print(d.get_header_properties())

print(d.get_signal_properties('XIRXRFMIXTRIM0.XM1PDAC1.XMN.MAIN'))
