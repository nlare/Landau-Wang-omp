#!/bin/bash
gnuplot test_g/Hist-L=8_PP=8-7/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=8_PP=8-7/graphs/{1..200}.jpg animate-Hist-L=8_PP=8-7.gif
