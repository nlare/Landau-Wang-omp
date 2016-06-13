#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/TermodinamicalStat_L=32_PP=2/St.jpg"
set grid x y
set xlabel "T"
set ylabel "St"
plot "results/TermodinamicalStat_L=3D_32_PP=2_MAXMCS=1000000.dat" using 1:4 title "landau-wang-omp-32" with lines lt rgb "red"