#!/bin/bash
gnuplot test_g/Hist-L=64_PP=8-5/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=64_PP=8-5/graphs/{1..200}.jpg animate-Hist-L=64_PP=8-5.gif
