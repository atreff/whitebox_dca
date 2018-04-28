#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "aes.h"
#include "utils.h"

namespace dca {

static unsigned int selection_function(uint8_t input, uint8_t keyguess, unsigned int bitmask) {
  unsigned int tmp = aes::sbox[keyguess ^ input];
  return (tmp >> bitmask) & 0x01;
}

int bestbit = -1;
int glob_maximum = -1;

// TODO documentation
void extract_key_byte(unsigned int byte, config_t &conf, unsigned int correct_key) {
  const unsigned int samples = conf.sample_end - conf.sample_start;

  std::array<std::array<std::pair<double, unsigned int>, 8>, 256> computed_values2;
  for (unsigned int key = 0; key < 256; ++key) {
    std::vector<unsigned int> group1_ctr(8, 0), group2_ctr(8, 0);
    std::vector<std::vector<double>> group1(8,
                                            std::vector<double>(samples, 0.0)),
        group2(8, std::vector<double>(samples, 0.0));
    for (unsigned int guess = 0; guess < conf.traces; ++guess) {
      for (unsigned int bitmask = 0; bitmask < 8; ++bitmask) {
        auto selection = selection_function(
            conf.guess_values.at(guess + byte), key, bitmask);
        if (selection == 1) {
          group1_ctr[bitmask]++;
          for (unsigned int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl) {
            group1[bitmask].at(smpl - conf.sample_start) +=
                conf.trace_values.at(guess * conf.samples_per_trace + smpl);
          }
        } else {
          group2_ctr[bitmask]++;
          for (unsigned int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl) {
            group2[bitmask].at(smpl - conf.sample_start) +=
                conf.trace_values.at(guess * conf.samples_per_trace + smpl);
          }
        }
      }
    }
    for (unsigned int bitmask = 0; bitmask < 8; ++bitmask) {
      for (size_t sample = 0; sample < group1[bitmask].size() && sample < group2[bitmask].size(); ++sample) {
        group1[bitmask].at(sample) /= group1_ctr[bitmask];
        group2[bitmask].at(sample) /= group2_ctr[bitmask];
      }
      auto sample_limit = std::min(group1[bitmask].size(), group2[bitmask].size());
      double maximum_val = -1.0;
      for (size_t sample = 0; sample < sample_limit; ++sample) {
        double diff_of_means_at_sample = std::fabs(group1[bitmask].at(sample) - group2[bitmask].at(sample));
        if (diff_of_means_at_sample > maximum_val) {
            // set new maximum value for this difference of means trace
            // when using the corresponding bit
          computed_values2[key].at(bitmask) = std::make_pair(diff_of_means_at_sample, key);
          maximum_val = diff_of_means_at_sample;
        }
        if (maximum_val > glob_maximum) {

          glob_maximum = maximum_val;
          bestbit = bitmask;
        }
      }
    }
  }
  for (unsigned int bitmask = 0; bitmask < 8; ++bitmask) {
    for (unsigned int keyguess = 0; keyguess < 256; ++keyguess) {
      std::sort(std::begin(computed_values2[keyguess]),
                std::end(computed_values2[keyguess]),
                [](std::pair<double, unsigned int> &fst, std::pair<double, unsigned int> &snd) {
        return fst.first > snd.first;
      });
    }
  }
  int bestbit_max = -1;
  std::vector<std::pair<unsigned int, double>> maximum_per_guess;
  std::pair<double, int> gmax = std::pair<double,int>(-1,-1);
  for (unsigned int keyguess = 0; keyguess < 256; keyguess++) {
    int bestb = -1;
    std::pair<double, int> currentMax = std::pair<double, int>(-1, -1);
    for (unsigned int bit = 0; bit < 8; bit++) {
      if (computed_values2[keyguess].at(bit).first > currentMax.first) {
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
  for(unsigned int keyguess = 0; keyguess < 256; keyguess++)
  {
  //    std::cout << keyguess << " -> " << maximum_per_guess[keyguess].second << std::endl;
  }
  std::cout << std::hex << "0x" << maximum_per_guess[0].first // best guess
      << " " << bestbit_max // best bit
      << " " << maximum_per_guess[0].second // best peak
      << " " << std::dec << position_of_correct_key+1 // correct position
      << " " << maximum_per_guess[position_of_correct_key].second << "\n"; // correct peak
}
} // namespace dca
