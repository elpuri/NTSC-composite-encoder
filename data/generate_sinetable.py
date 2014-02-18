#!/usr/bin/python

import os

amplitude = 150
length = 256
width = 9
steps = 7
amplitude_step = amplitude / steps

for i in range(1, steps + 1) :
    cmd = "romswak sine -width {0} -length {1} -amplitude {2} -o sine{3}.bin -signed".format(width, length, amplitude, i)
    amplitude = int(amplitude - amplitude_step)
    print cmd
    os.system(cmd)

makemif = "romswak data "
for i in range(1, steps + 1) :
    makemif = makemif + "sine{0}.bin ".format(i)
makemif = makemif + "-o color_carrier_sine.mif -mif -width {0}".format(width)
print makemif
os.system(makemif)

rm = "rm sine*.bin"
os.system(rm)

