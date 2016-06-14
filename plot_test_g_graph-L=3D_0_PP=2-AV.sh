#!/bin/bash
gnuplot test_g/DoS-L=0_PP=2-AV/temp/*.plot
convert -delay 10 -loop 0 test_g/DoS-L=0_PP=2-AV/graphs/{1..20}.jpg animate-DoS-L=0_PP=2-AV.gif
