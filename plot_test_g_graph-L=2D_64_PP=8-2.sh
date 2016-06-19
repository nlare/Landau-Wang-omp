#!/bin/bash
gnuplot test_g/DoS-L=64_PP=8-2/temp/*.plot
convert -delay 10 -loop 0 test_g/DoS-L=64_PP=8-2/graphs/{1..20}.jpg animate-DoS-L=64_PP=8-2.gif
