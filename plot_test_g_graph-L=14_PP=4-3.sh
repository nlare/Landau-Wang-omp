#!/bin/bash
gnuplot test_g/DoS-L=14_PP=4-3/temp/*.plot
convert -delay 10 -loop 0 test_g/DoS-L=14_PP=4-3/graphs/{1..20}.jpg animate-DoS-L=14_PP=4-3.gif
