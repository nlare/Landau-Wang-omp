#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/TermodinamicalStat_L=16_PP=2/Ct.jpg"
set grid x y
set xlabel "T"
set ylabel "Ct"
plot "results/TermodinamicalStat_L=3D_16_PP=2_MAXMCS=1000000.dat" using 1:5 title "landau-wang-omp-16" with lines lt rgb "red"