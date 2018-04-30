load 'common.gp'

set xlabel "samples"
set yrange [0:1]
#set ytics (0, .005, .01)
set ylabel "difference of means"

#set xrange [0:8]
#set format x "0x%X"
#set xtics (0, 4, 8)
set output "hard-tboxlin-correct.pdf"
plot "data/sample_data_single_tbox_linear_strong" u 0:256 notitle with steps ls 1
