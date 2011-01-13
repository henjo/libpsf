import psf

d = psf.PSFDataSet("/nfs/home/henrik/pycircuit.repo/pycircuit/post/cds/test/psf/tran.tran") 


#"/nfs/home/henrik/spectre/1/pnoise.raw/pnoise_pout3g.pnoise")

print list(d.get_signal_names())

print d.get_signal_values('net21')


print d.get_param_values()
#print d.get_signal_values('tx_iqfilter_stop_0.tx_iqfilter_stoplyr1_0.I23.mn4')

#stotal = 0
#for name in d.get_signal_names():
#    if name != 'out':
#        stotal += d.get_signal_values(name)[3]['total']#

#print stotal
