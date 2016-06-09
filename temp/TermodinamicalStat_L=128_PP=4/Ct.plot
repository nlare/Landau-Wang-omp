#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "graph/TermodinamicalStat_L=128_PP=4/Ct.jpg"
set grid x y
set xlabel "T"
set ylabel "Ct"
plot "results/TermodinamicalStat_L=128_PP=4_MAXMCS=1000000.dat" using 1:5 title "landau-wang-omp-128" with lines lt rgb "red"