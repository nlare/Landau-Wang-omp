#!/bin/bash
gnuplot test_g/Hist-L=4_PP=2-AV/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=4_PP=2-AV/graphs/{1..200}.jpg animate-Hist-L=4_PP=2-AV.gif
