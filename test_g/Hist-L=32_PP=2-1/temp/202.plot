#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=32_PP=2-1/graphs/202.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [256:2048]
set yrange [0:20000]
set xlabel "i [256:2048]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=32_PP=2-1/202.dat" using 1:3 title "landau-wang-32-iteration-202" with lines lt rgb "red"#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/Hist-L=32_PP=2-1/graphs/202.jpg"
set grid x y
set xtics 10 
set ytics 2000 
set mxtics 10 
set mytics 10  
set xrange [256:2048]
set yrange [0:20000]
set xlabel "i [256:2048]"
set ylabel "Hist(i)"
plot "test_g/Hist-L=32_PP=2-1/202.dat" using 1:3 title "landau-wang-32-iteration-202" with lines lt rgb "red"#!/usr/bin/gnuplot -persist
set terminal jpeg font arial 12 size 800,600
set output "test_g/DoS-L=32_PP=2-0/graphs/1.jpg"
set grid x y
set xtics 20 
set ytics 1000 
set mxtics 5 
set mytics 5  
set xrange [0:1792]
set xlabel "i [0:1792]"
set ylabel "G(i)"
plot "test_g/DoS-L=32_PP=2-0/1.dat" using 1:3 title "landau-wang-32-iteration-1" with lines lt rgb "red"