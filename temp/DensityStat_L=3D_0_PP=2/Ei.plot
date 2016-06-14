#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/DensityStat_L=0_PP=2/Ei.jpg"
set grid x y
set xlabel "i"
set ylabel "E(i)"
plot "results/DensityStat_L=3D_0_PP=2_MAXMCS=100000.dat" using 1:2 title "landau-wang-omp-0" with lines lt rgb "red"