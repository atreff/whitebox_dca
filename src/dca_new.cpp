#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "aes.h"
#include "utils.h"

namespace dca {
    
typedef double Sample;
typedef std::vector<Sample> Trace;

static unsigned int selection_function(uint8_t input, uint8_t keyguess, unsigned int bit)
{
    unsigned int tmp = aes::sbox[keyguess ^ input];
    return (tmp >> bit) & 0x01;
}

int bestbit = -1;
int glob_maximum = -1;

// TODO documentation
void extract_key_byte(unsigned int byte, config_t &conf, unsigned int correct_key)
{
    const unsigned int samples = conf.sample_end - conf.sample_start;

    std::array<std::array<std::pair<double, unsigned int>, 8>, 256> computed_values2;
    for (unsigned int key = 0; key < 256; ++key)
    {
        std::array<std::array<unsigned int, 8>, 2> group_ctrs;
        std::vector<std::vector<Trace>> groups;
        groups.emplace_back(8, Trace(samples, 0.0));
        groups.emplace_back(8, Trace(samples, 0.0));
    
        for (unsigned int trace = 0; trace < conf.traces; ++trace)
        {
            for (unsigned int bit = 0; bit < 8; ++bit)
            {
                auto sel = selection_function(conf.guess_values.at(trace + byte), key, bit);
                group_ctrs[sel][bit]++;
                for (unsigned int sample = conf.sample_start; sample < conf.sample_end; ++sample)
                {
                    groups[sel][bit][sample - conf.sample_start] +=
                        conf.trace_values[trace * conf.samples_per_trace + sample];
                }
            }
        }
        for (unsigned int bit = 0; bit < 8; ++bit)
        {
            for (size_t sample = 0; sample < groups[0][bit].size() && sample < groups[1][bit].size(); ++sample)
            {
                groups[0][bit].at(sample) /= group_ctrs[0][bit];
                groups[1][bit].at(sample) /= group_ctrs[1][bit];
            }
            auto sample_limit = std::min(groups[0][bit].size(), groups[1][bit].size());
            double maximum_val = -1.0;
            for (size_t sample = 0; sample < sample_limit; ++sample)
            {
                double diff_of_means_at_sample = std::fabs(groups[0][bit].at(sample) - groups[1][bit].at(sample));
                if (diff_of_means_at_sample > maximum_val)
                {
                    // set new maximum value for this difference of means trace
                    // when using the corresponding bit
                    computed_values2[key].at(bit) = std::make_pair(diff_of_means_at_sample, key);
                    maximum_val = diff_of_means_at_sample;
                }
                if (maximum_val > glob_maximum)
                {
                    glob_maximum = maximum_val;
                    bestbit = bit;
                }
            }
        }
    }
    for (unsigned int bit = 0; bit < 8; ++bit)
    {
        for (unsigned int keyguess = 0; keyguess < 256; ++keyguess)
        {
            std::sort(std::begin(computed_values2[keyguess]),
                      std::end(computed_values2[keyguess]),
                      [](std::pair<double, unsigned int> &fst, std::pair<double, unsigned int> &snd)
                      {
                          return fst.first > snd.first;
                      });
        }
    }
    int bestbit_max = -1;
    std::vector<std::pair<unsigned int, double>> maximum_per_guess;
    std::pair<double, int> gmax = std::pair<double,int>(-1,-1);
    for (unsigned int keyguess = 0; keyguess < 256; keyguess++)
    {
        int bestb = -1;
        std::pair<double, int> currentMax = std::pair<double, int>(-1, -1);
        for (unsigned int bit = 0; bit < 8; bit++)
        {
            if (computed_values2[keyguess].at(bit).first > currentMax.first)
            {
                currentMax = computed_values2[keyguess].at(bit);
                bestb = bit;
                bestbit_max = bit;
            }
        }
        maximum_per_guess.push_back(std::pair<int, double>(keyguess, currentMax.first));
    }

    for(unsigned int keyguess = 0; keyguess < 256; ++keyguess)
    {
        std::sort(std::begin(computed_values2[keyguess]),
              std::end(computed_values2[keyguess]),
              [](std::pair<double, unsigned int> &fst, std::pair<double, unsigned int> &snd) {
                  return fst.second > snd.second;
               });
    }
    std::ofstream ofs("stats.out");
    for(unsigned int keyguess = 0; keyguess < 256; keyguess++)
    {
        ofs << keyguess << " " << maximum_per_guess[keyguess].second << std::endl;
        std::cout << keyguess << " -> " << maximum_per_guess[keyguess].second << std::endl;
    }
    std::sort(std::begin(maximum_per_guess), std::end(maximum_per_guess),
          [](std::pair<unsigned int, double>& fst, std::pair<unsigned int, double>& snd)
          {
            return fst.second > snd.second;
          });
    int position_of_correct_key = -1;
    for(unsigned int keyguess = 0; keyguess < 256; keyguess++)
    {
        if(maximum_per_guess[keyguess].first == correct_key)
        {
            position_of_correct_key = keyguess;
            break;
        }
    }
    std::cout << std::hex << "0x" << maximum_per_guess[0].first // best guess
      << " " << bestbit_max // best bit
      << " " << maximum_per_guess[0].second // best peak
      << " " << std::dec << position_of_correct_key+1 // correct position
      << " " << maximum_per_guess[position_of_correct_key].second << "\n"; // correct peak
}

} // namespace dca
