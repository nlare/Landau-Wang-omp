#!/bin/bash
gnuplot test_g/DoS-L=8_PP=16-14/temp/*.plot
convert -delay 10 -loop 0 test_g/DoS-L=8_PP=16-14/graphs/{1..20}.jpg animate-DoS-L=8_PP=16-14.gif
