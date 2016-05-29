#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=32_PP=8-3/graphs/1396.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [38:1996]
set yrange [0:20000]
set xlabel "i [38:1996]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=32_PP=8-3/1396.dat" using 1:3 title "landau-wang-32-iteration-1396" with lines lt rgb "red"