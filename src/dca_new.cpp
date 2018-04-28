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

// TODO documentation
void extract_key_byte(unsigned int byte, config_t &conf, unsigned int correct_key)
{
    const unsigned int samples = conf.sample_end - conf.sample_start;

    std::array<std::array<std::pair<double, unsigned int>, 8>, 256> peak_for_key;
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
                auto sel = selection_function(conf.guess_values[trace + byte], key, bit);
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
                groups[0][bit][sample] /= group_ctrs[0][bit];
                groups[1][bit][sample] /= group_ctrs[1][bit];
            }
            auto max_samples = std::min(groups[0][bit].size(), groups[1][bit].size());
            double maximum_dom = -1.0;
            for (size_t sample = 0; sample < max_samples; ++sample)
            {
                double sample_diff = std::fabs(groups[0][bit][sample] - groups[1][bit][sample]);
                if (sample_diff > maximum_dom)
                {
                    // set new maximum value for this difference of means trace
                    // when using the corresponding bit
                    peak_for_key[key][bit] = std::make_pair(sample_diff, key);
                    maximum_dom = sample_diff;
                }
            }
        }
    }
    for (unsigned int bit = 0; bit < 8; ++bit)
    {
        for (unsigned int keyguess = 0; keyguess < 256; ++keyguess)
        {
            std::sort(std::begin(peak_for_key[keyguess]),
                      std::end(peak_for_key[keyguess]),
                      [](std::pair<double, unsigned int> &fst, std::pair<double, unsigned int> &snd)
                      {
                          return fst.first > snd.first;
                      });
        }
    }
    
    // calculate the highest peak for each key guess
    int bestbit_max = -1;
    std::vector<std::pair<unsigned int, double>> maximum_per_guess; // maps from key guess => best peak over all 8 bits
    for (unsigned int keyguess = 0; keyguess < 256; keyguess++)
    {
        std::pair<double, int> current_peak = std::pair<double, int>(-1, -1);
        for (unsigned int bit = 0; bit < 8; bit++)
        {
            if (peak_for_key[keyguess][bit].first > current_peak.first)
            {
                current_peak = peak_for_key[keyguess][bit];
                bestbit_max = bit;
            }
        }
        maximum_per_guess.push_back(std::pair<int, double>(keyguess, current_peak.first));
    }
    // end: calculate the highest peak for each key guess
    

    for(unsigned int keyguess = 0; keyguess < 256; ++keyguess)
    {
        std::sort(std::begin(peak_for_key[keyguess]),
              std::end(peak_for_key[keyguess]),
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
          
    // look for the position of the given correct key
    // if you give a wrong key, you'll get the position of that instead
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
