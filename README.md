# How to compile
You need a C++ compiler capable of C++11.
If you're using g++, the compilation step may look like:
`g++ -std=c++11 main.cpp utils.cpp dca.cpp -O2 -o dca` for the original DCA attack or
`g++ -std=c++11 main.cpp utils.cpp dca_new.cpp -O2 -o dca-new` for the updated DCA attack.

# Usage
1. Use existing tools like the `trace_it.py` script and `daredevil` from the [SideChannelMarvels](https://github.com/SideChannelMarvels) to obtain software execution traces (you need the .input and .trace files)
1. if you're interesting in persistent stored output data, create a directory called `graph_out`.
1. run `./dca[-new] <tracename.trace> <tracename.input> <number of traces> <byte>`
1. after the run, you'll find the corresponding output data in `graph_out/`
1. now you can process these data files using the gnuplot-scripts that can be found in the `gnuplot/` folder. You have to modify them by yourself to match the correct file names for input and output (and maybe you would like to limit the range etc.)

# References
You may want to have a look at https://eprint.iacr.org/2018/301.pdf to see the generated traces in action.
