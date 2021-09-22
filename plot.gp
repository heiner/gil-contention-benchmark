set datafile separator "\t"

set style line 1 lc rgb "red"
set style line 2 lc rgb "blue"

set boxwidth 0.5
set style fill solid

set xtic nomirror offset 3,0 noenhanced
set key autotitle columnheader noenhanced

set boxwidth 0.25

set yrange [0:*]

plot 'data.dat' using ($0-0.25):2:xtic(1) with boxes ls 1, \
  '' using ($0-0.25):2:3 with yerrorbars lc rgb 'black' lw 2, \
  '' using 0:4 with boxes ls 1 lc rgb 'blue', \
  '' using 0:4:5 with yerrorbars lc rgb 'black' lw 2

pause(-1)
