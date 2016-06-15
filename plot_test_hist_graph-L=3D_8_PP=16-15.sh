#!/bin/bash
gnuplot test_g/Hist-L=8_PP=16-15/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=8_PP=16-15/graphs/{1..200}.jpg animate-Hist-L=8_PP=16-15.gif
