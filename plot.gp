
set style line 1 lc rgb "red"
set style line 2 lc rgb "blue"

set boxwidth 0.5
set style fill solid

plot '-' using 0:2:xtic(1) with boxes ls 1, '-' using 0:1:2 with yerrorbars lc rgb 'black' lw 2
"single GIL" 3561092.83 256499.85
"multi GIL" 3618501.08 802961.93
"single drop GIL" 2978816.49 291618.80
"multi drop GIL" 281212.77 233375.12
e
3561092.83 256499.85
3618501.08 802961.93
2978816.49 291618.80
281212.77 233375.12
e

pause(-1)
