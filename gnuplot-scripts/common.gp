set terminal pdfcairo \
    size 9in,2.2in \
    font "Optima,12" \
    linewidth 2 \
    rounded \
    fontscale 1.0 \
    enhanced

# Line style for axes
set style line 80 lt rgb "#808080"

# Line style for grid
set style line 81 lt 0  # dashed
set style line 81 lt rgb "#cfcfcf"  # grey

set grid back linestyle 81
set border 3 back linestyle 80
set xtics nomirror font ",9"
set ytics nomirror font ",9"

set style line 1 lt rgb "#5060D0" lw 1 pt 1

set key top right

set ytics 0.25
