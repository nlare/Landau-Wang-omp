#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=128_PP=2-0/graphs/188.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [0:28672]
set yrange [0:20000]
set xlabel "i [0:28672]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=128_PP=2-0/188.dat" using 1:3 title "landau-wang-128-iteration-188" with lines lt rgb "red"