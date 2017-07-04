#include <iomanip>
#include <iostream>
#include <thread>
#include "dpa.h"
#include "utils.h"

// TODO documentation

int main(int argc, char *argv[])
{
    if(argc < 5)
    {
        std::cerr << "Usage: " << argv[0] << " <trace_file> <guess_file> <num_traces> <num_threads>\n";
        return 1;
    }
    config_t conf;
    // TODO error handling
    conf.trace_values = load_file(argv[1]);
    conf.guess_values = load_file(argv[2]);
    conf.traces = std::atoi(argv[3]);
    auto num_threads = std::atoi(argv[4]);
    const int samples = conf.trace_values.size() / conf.traces;

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
                extract_key_byte(byte, samples, conf);
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

