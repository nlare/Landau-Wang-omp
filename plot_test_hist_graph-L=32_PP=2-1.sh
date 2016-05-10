#!/bin/bash
gnuplot test_g/Hist-L=32_PP=2-1/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=32_PP=2-1/graphs/{1..200}.jpg animate-Hist-L=32_PP=2-1.gif
