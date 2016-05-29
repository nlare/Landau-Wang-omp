#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/DoS-L=32_PP=8-7/graphs/1.jpg"
set grid x y
set xtics 20 
set ytics 1000 
set mxtics 5 
set mytics 5  
set xrange [90:2048]
set xlabel "i [90:2048]"
set ylabel "G(i)"
plot "test_g/DoS-L=32_PP=8-7/1.dat" using 1:3 title "landau-wang-32-iteration-1" with lines lt rgb "red"