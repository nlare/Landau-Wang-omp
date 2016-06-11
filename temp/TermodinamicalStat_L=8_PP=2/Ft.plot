#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/TermodinamicalStat_L=8_PP=2/Ft.jpg"
set grid x y
set xlabel "T"
set ylabel "Ft"
plot "results/TermodinamicalStat_L=3D_8_PP=2_MAXMCS=1000000.dat" using 1:3 title "landau-wang-omp-8" with lines lt rgb "red"