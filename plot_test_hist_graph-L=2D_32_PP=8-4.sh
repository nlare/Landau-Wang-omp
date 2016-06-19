#!/bin/bash
gnuplot test_g/Hist-L=32_PP=8-4/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=32_PP=8-4/graphs/{1..200}.jpg animate-Hist-L=32_PP=8-4.gif
