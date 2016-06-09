#!/bin/bash
gnuplot test_g/DoS-L=16_PP=8-7/temp/*.plot
convert -delay 10 -loop 0 test_g/DoS-L=16_PP=8-7/graphs/{1..20}.jpg animate-DoS-L=16_PP=8-7.gif
