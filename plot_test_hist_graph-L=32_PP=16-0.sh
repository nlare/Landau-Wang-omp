#!/bin/bash
gnuplot test_g/Hist-L=32_PP=16-0/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=32_PP=16-0/graphs/{1..200}.jpg animate-Hist-L=32_PP=16-0.gif
