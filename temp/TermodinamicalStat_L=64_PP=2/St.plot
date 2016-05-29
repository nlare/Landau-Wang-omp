#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/TermodinamicalStat_L=64_PP=2/St.jpg"
set grid x y
set xlabel "T"
set ylabel "St"
plot "results/TermodinamicalStat_L=64_PP=2.dat" using 1:4 title "landau-wang-omp-64" with lines lt rgb "red"