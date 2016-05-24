#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/DoS-L=16_PP=4-2/graphs/1.jpg"
set grid x y
set xtics 20 
set ytics 1000 
set mxtics 5 
set mytics 5  
set xrange [12:506]
set xlabel "i [12:506]"
set ylabel "G(i)"
plot "test_g/DoS-L=16_PP=4-2/1.dat" using 1:3 title "landau-wang-16-iteration-1" with lines lt rgb "red"