#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=64_PP=2-1/graphs/46.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [1024:8192]
set yrange [0:20000]
set xlabel "i [1024:8192]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=64_PP=2-1/46.dat" using 1:3 title "landau-wang-64-iteration-46" with lines lt rgb "red"