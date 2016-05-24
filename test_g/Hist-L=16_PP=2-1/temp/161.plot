#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=16_PP=2-1/graphs/161.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [12:512]
set yrange [0:20000]
set xlabel "i [12:512]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=16_PP=2-1/161.dat" using 1:3 title "landau-wang-16-iteration-161" with lines lt rgb "red"