#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/DensityStat_L=8_PP=4/gi.jpg"
set grid x y
set xlabel "i"
set ylabel "g(i)"
plot "results/DensityStat_L=3D_8_PP=4_MAXMCS=100000.dat" using 1:3 title "landau-wang-omp-8" with lines lt rgb "red"