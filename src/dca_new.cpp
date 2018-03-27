#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "aes.h"
#include "utils.h"

namespace dca {

static int selection_function(uint8_t input, uint8_t keyguess, int bitmask) {
  int tmp = aes::sbox[keyguess ^ input];
  return (tmp >> bitmask) & 0x01;
}

int bestbit = -1;
int glob_maximum = -1;

// TODO documentation
void extract_key_byte(int byte, config_t &conf, int correct_key) {
  const int samples = conf.sample_end - conf.sample_start;
  std::vector<std::vector<std::vector<double>>> attack_graphs(
      8,
      std::vector<std::vector<double>>(256, std::vector<double>(samples, 0.0)));

  std::array<std::array<std::pair<double, int>, 8>, 256> computed_values2;
  for (int key = 0; key < 256; ++key) {
    std::vector<int> group1_ctr(8, 0), group2_ctr(8, 0);
    std::vector<std::vector<double>> group1(8,
                                            std::vector<double>(samples, 0.0)),
        group2(8, std::vector<double>(samples, 0.0));
    for (int guess = 0; guess < conf.traces; ++guess) {
      for (int bitmask = 0; bitmask < 8; ++bitmask) {
        auto selection = selection_function(
            conf.guess_values.at(guess + byte), key, bitmask);
        if (selection == 1) {
          group1_ctr[bitmask]++;
          for (int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl) {
            group1[bitmask].at(smpl - conf.sample_start) +=
                conf.trace_values.at(guess * conf.samples_per_trace + smpl);
          }
        } else {
          group2_ctr[bitmask]++;
          for (int smpl = conf.sample_start; smpl < conf.sample_end; ++smpl) {
            group2[bitmask].at(smpl - conf.sample_start) +=
                conf.trace_values.at(guess * conf.samples_per_trace + smpl);
          }
        }
      }
    }
    for (int bitmask = 0; bitmask < 8; ++bitmask) {
      for (size_t g = 0; g < group1[bitmask].size(); ++g) {
        group1[bitmask].at(g) /= group1_ctr[bitmask];
      }
      for (size_t g = 0; g < group2[bitmask].size(); ++g) {
        group2[bitmask].at(g) /= group2_ctr[bitmask];
      }
      auto limit = std::min(group1[bitmask].size(), group2[bitmask].size());
      double maximum_val = -1.0;
      for (size_t g = 0; g < limit; ++g) {
        double tmp = std::fabs(group1[bitmask].at(g) - group2[bitmask].at(g));
        attack_graphs[bitmask].at(key).at(g) = tmp;
        if (tmp > maximum_val) {
          computed_values2[key].at(bitmask) = std::make_pair(tmp, key);
          maximum_val = tmp;
        }
        if (maximum_val > glob_maximum) {

          glob_maximum = maximum_val;
          bestbit = bitmask;
        }
      }
    }
  }
  for (int bitmask = 0; bitmask < 8; ++bitmask) {
    for (int key = 0; key < 256; ++key) {
      std::sort(std::begin(computed_values2[key]),
                std::end(computed_values2[key]),
                [](std::pair<double, int> &fst, std::pair<double, int> &snd) {
        return fst.first > snd.first;
      });
    }
  }
  int bestb_max = -1;
  std::vector<std::pair<int, double>> maximum_per_guess;
  std::pair<double, int> gmax = std::pair<double,int>(-1,-1);
  for (int ik = 0; ik < 256; ik++) {
    int bestb = -1;
    std::pair<double, int> currentMax = std::pair<double, int>(-1, -1);
//    std::cout << "Key " << std::hex << ik << ": [";
    for (int jb = 0; jb < 8; jb++) {
//        std::cout << computed_values2[ik].at(jb).first;
//        if(jb < 7) { std::cout << ','; }
      if (computed_values2[ik].at(jb).first > currentMax.first) {
        currentMax = computed_values2[ik].at(jb);
        bestb = jb;
        bestb_max = jb;
      }
    }
    maximum_per_guess.push_back(std::pair<int, double>(ik, currentMax.first));
//    std::cout << "], best: " << currentMax.first << " (bit " << bestb
//              << ")\n";
  }

  for(int key = 0; key < 256; ++key)
  {
  std::sort(std::begin(computed_values2[key]),
          std::end(computed_values2[key]),
          [](std::pair<double, int> &fst, std::pair<double, int> &snd) {
          return fst.second > snd.second;
          });
  }
  std::ofstream ofs("stats.out");
  for(int i = 0; i < 256; i++)
  {
      ofs << i << " " << maximum_per_guess[i].second << std::endl;
      std::cout << i << " -> " << maximum_per_guess[i].second << std::endl;
  }
  std::sort(std::begin(maximum_per_guess), std::end(maximum_per_guess),
          [](std::pair<int, double>& fst, std::pair<int, double>& snd)
          {
            return fst.second > snd.second;
          });
  int pos = -1;
  for(int i = 0; i < 256; i++)
  {
      if(maximum_per_guess[i].first == correct_key)
      {
          pos = i;
          break;
      }
  }
  for(int i = 0; i < 256; i++)
  {
  //    std::cout << i << " -> " << maximum_per_guess[i].second << std::endl;
  }
  std::cout << std::hex << "0x" << maximum_per_guess[0].first // best guess
      << " " << bestb_max // best bit
      << " " << maximum_per_guess[0].second // best peak
      << " " << std::dec << pos+1 // correct position
      << " " << maximum_per_guess[pos].second << "\n"; // correct peak
}
} // namespace dca
