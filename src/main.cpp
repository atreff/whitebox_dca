#include <iomanip>
#include <iostream>
#include <thread>
#include "dca.h"
#include "utils.h"

// TODO documentation

int main(int argc, char *argv[])
{
    if(argc < 6)
    {
        std::cerr << "Usage: " << argv[0]
            << " <trace_file> <guess_file> <num_traces> <num_threads> <attacked_bit> (<sample_start> <sample_end>)\n";
        return 1;
    }

    dca::config_t conf;
    // TODO error handling
    conf.trace_values = dca::utils::load_file(argv[1]);
    conf.guess_values = dca::utils::load_file(argv[2]);
    conf.traces = std::atoi(argv[3]);
    auto num_threads = std::atoi(argv[4]);
    conf.samples_per_trace = conf.trace_values.size() / conf.traces;
    conf.sample_start = 0;
    conf.sample_end = conf.samples_per_trace;
    int bitmask = std::atoi(argv[5]);
    if(argc > 7)
    {
        conf.sample_start = std::atoi(argv[6]);
        conf.sample_end   = std::atoi(argv[7]);
    }

    if(num_threads < 1 || num_threads % 2 != 0)
    {
        num_threads = 1;
    }
    std::vector<std::thread> worker_threads(num_threads);
    for(size_t thread = 0; thread < worker_threads.size(); ++thread)
    {
        worker_threads.at(thread) = std::thread(
                [&](int thr)
                {
                auto chunk_size = conf.solved_key.size() / worker_threads.size();
                int start = chunk_size * thr;
                int stop = start + chunk_size;
                for(int byte = start; byte < stop; ++byte)
                {
                    dca::extract_key_byte(byte, conf, bitmask);
                }
                }, thread);
    }

    for(auto &t : worker_threads)
    {
        t.join();
    }

    std::cout << std::setfill('0') << std::hex << '\n';
    for(auto &key : conf.solved_key)
    {
        std::cout << std::setw(2) << static_cast<unsigned int>(key) << ' ';
    }
    std::cout << '\n';

    return 0;
}

