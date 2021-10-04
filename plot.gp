set datafile separator "\t"

set style line 1 lc rgb "red"
set style line 2 lc rgb "blue"

set boxwidth 0.5
set style fill solid

set xtic nomirror offset 3,0 noenhanced
set key autotitle columnheader noenhanced

set boxwidth 0.25

set yrange [0:*]
# set logscale y
set format y "%.0fK"

set bmargin 3

set title "calls/sec (in K)"

plot 'data.dat' using ($0-0.25):2:xtic(1) with boxes ls 1, \
  '' using ($0-0.25):0:2 with labels offset -1,-2.0 title "", \
  '' using 0:3 with boxes ls 1 lc rgb 'blue', \
  '' using 0:0:3 with labels offset 1,-2.0 title "", \


pause(-1)
