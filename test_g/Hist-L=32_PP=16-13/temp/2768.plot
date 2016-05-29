#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=32_PP=16-13/graphs/2768.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [84:2036]
set yrange [0:20000]
set xlabel "i [84:2036]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=32_PP=16-13/2768.dat" using 1:3 title "landau-wang-32-iteration-2768" with lines lt rgb "red"