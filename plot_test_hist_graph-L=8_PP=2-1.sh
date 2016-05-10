#!/bin/bash
gnuplot test_g/Hist-L=8_PP=2-1/temp/*.plot
convert -delay 10 -loop 0 test_g/Hist-L=8_PP=2-1/graphs/{1..100}.jpg animate-Hist-L=8_PP=2-1.gif
