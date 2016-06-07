#!/bin/bash
gnuplot test_g/Hist-L=14_PP=4-0/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=14_PP=4-0/graphs/{1..200}.jpg animate-Hist-L=14_PP=4-0.gif
