#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/DensityStat_L=16_PP=2/Ei.jpg"
set grid x y
set xlabel "i"
set ylabel "E(i)"
plot "results/DensityStat_L=16_PP=2_MAXMCS=10000.dat" using 1:2 title "landau-wang-omp-16" with lines lt rgb "red"