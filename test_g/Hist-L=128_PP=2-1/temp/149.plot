#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=128_PP=2-1/graphs/149.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [4096:32768]
set yrange [0:20000]
set xlabel "i [4096:32768]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=128_PP=2-1/149.dat" using 1:3 title "landau-wang-128-iteration-149" with lines lt rgb "red"