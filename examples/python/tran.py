import matplotlib.pyplot as plt
import libpsf

d = libpsf.PSFDataSet("../data/timeSweep")
print(d.get_header_properties())


print(list(d.get_signal_names()))
t = d.get_sweep_values()
v = d.get_signal('INP')

print (len(v), len(t))

plt.plot(t, v)
plt.show()

