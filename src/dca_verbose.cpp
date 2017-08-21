#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "aes.h"
#include "utils.h"

namespace dca {

static int selection_function(uint8_t input, uint8_t keyguess, int bitmask)
{
    int tmp = aes::sbox[keyguess ^ input];
    return (tmp >> bitmask) & 0x01;
}

// TODO documentation
void extract_key_byte(int byte, config_t& conf, int bitmask)
{
    const int samples = conf.sample_end - conf.sample_start;
    std::vector<std::vector<double> > attack_graphs(256, std::vector<double>(samples, 0.0));
    bool written_single_trace = false;
    std::ofstream ofs("graph_out/single_trace", std::ios::out);
    std::array<std::pair<double, int>, 256> computed_values;
    for(int key = 0; key < 256; ++key)
    {
        int group1_ctr = 0, group2_ctr = 0;
        std::vector<double> group1(samples, 0.0), group2(samples, 0.0);
        for(int guess = 0; guess < conf.traces; ++guess)
        {
            auto selection = selection_function(
				conf.guess_values.at(16 * guess + byte), key, bitmask);
            if(!written_single_trace)
            {
                std::cerr << "ofs.is_open: " << ofs.is_open() << '\n';
            }
            if(selection == 1)
            {
                group1_ctr++;
                for(int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl)
                {
                    group1.at(smpl-conf.sample_start) += conf.trace_values.at(guess * conf.samples_per_trace + smpl);
                }
            }
            else
            {
                group2_ctr++;
                for(int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl)
                {
                    group2.at(smpl-conf.sample_start) += conf.trace_values.at(guess * conf.samples_per_trace + smpl);
                    if(ofs.is_open())
                    {
                        ofs << (int)conf.trace_values.at(guess * conf.samples_per_trace + smpl) << '\n';
                    }
                    if(!written_single_trace)
                    {
                        ofs.close();
                        written_single_trace  = true;
                    }
                }
            }
        }
        for(size_t g = 0; g < group1.size(); ++g)
        {
            group1.at(g) /= group1_ctr;
        }
        for(size_t g = 0; g < group2.size(); ++g)
        {
            group2.at(g) /= group2_ctr;
        }
        auto limit = std::min(group1.size(), group2.size());
        double maximum_val = -1.0;
        for(size_t g = 0; g < limit; ++g)
        {
            double tmp = std::fabs(group1.at(g) - group2.at(g));
            attack_graphs.at(key).at(g) = tmp;
            if(tmp > maximum_val)
            {
                computed_values.at(key) = std::make_pair(tmp, key);
                maximum_val = tmp;
            }
        }

        // verbose/byte_[byte]_guess_[key] format:
        // mean_ones mean_zeros
        std::stringstream fname;
        fname << "graph_out/verbose/byte_" << byte << "_guess_" << std::hex << key;
        std::ofstream file(fname.str(), std::ios::out);

        for(int i = conf.sample_start; i < conf.sample_end; ++i)
        {
            file << group1.at(i) << ' ' << group2.at(i) << '\n';
        }

    }
    std::sort(std::begin(computed_values), std::end(computed_values), [](std::pair<double,int>& fst, std::pair<double,int>& snd){ return fst.first > snd.first; });
    for(int i = 0; i < 5; ++i)
    {
        std::cout << std::hex << computed_values.at(i).second << ':' << computed_values.at(i).first << ", ";
    }
    conf.solved_key.at(byte) = computed_values.at(0).second;
    std::cout << '\n';

    // write 16 best trace graphs per byte

    std::stringstream fname;
    fname << "graph_out/combined_all_" << byte;
    std::ofstream file(fname.str(), std::ios::out);

    for(int i = 0; i < samples; ++i)
    {
        for(int j = 0; j < 16; ++j)
        {
            file << attack_graphs.at(computed_values.at(j).second).at(i) << ' ';
        }
        file << '\n';
    }
}

} // namespace dca

