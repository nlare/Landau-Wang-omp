#!/bin/bash
gnuplot test_g/Hist-L=32_PP=16-3/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=32_PP=16-3/graphs/{1..200}.jpg animate-Hist-L=32_PP=16-3.gif
