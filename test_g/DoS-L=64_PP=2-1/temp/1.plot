#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/DoS-L=64_PP=2-1/graphs/1.jpg"
set grid x y
set xtics 20 
set ytics 1000 
set mxtics 5 
set mytics 5  
set xrange [204:8192]
set xlabel "i [204:8192]"
set ylabel "G(i)"
plot "test_g/DoS-L=64_PP=2-1/1.dat" using 1:3 title "landau-wang-64-iteration-1" with lines lt rgb "red"