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

static unsigned int selection_function(uint8_t input, uint8_t keyguess, unsigned int bitmask)
{
    unsigned int tmp = aes::sbox[keyguess ^ input];
    return (tmp >> bitmask) & 0x01;
}

// TODO encapsulate stuff in a class
static void normalize_mean_traces(std::vector<Trace>& mean_traces, std::array<unsigned int, 2> const & counter, unsigned int max_samples)
{
    for(size_t sample = 0; sample < max_samples; ++sample)
    {
        for(size_t i = 0; i < mean_traces.size(); ++i)
        {
            mean_traces[i].at(sample) /= counter[i];
        }
    }
}

void extract_key_byte(unsigned int byte, config_t& conf, unsigned int bitmask)
{
    const unsigned int samples = conf.sample_end - conf.sample_start;
    const unsigned int trace_len = conf.samples_per_trace;

    std::vector<Trace> dom_traces(0xff, Trace(samples, 0.0));

    std::array<std::pair<Sample, unsigned int>, 0xff> max_peak_per_keyguess;
    for(unsigned int keyguess = 0; keyguess < 0xff; ++keyguess)
    {
        std::vector<Trace> mean_traces;
        // add two traces, one for each selection outcome (0 or 1), each <samples> long, initialized with 0.0
        mean_traces.emplace_back(samples, 0.0);
        mean_traces.emplace_back(samples, 0.0);

        std::array<unsigned int, 2> counter;
        for(unsigned int traceid = 0; traceid < conf.traces; ++traceid)
        {
            // calculate hypothesis based on input (guess_values) and our keyguess
            auto selection = selection_function(
				conf.guess_values.at(16 * traceid + byte), keyguess, bitmask);
            ++counter[selection];

            // sort our traces into two sets of traces depending on the hypothesis
            for(unsigned int sample = conf.sample_start; sample < conf.sample_end; ++sample)
            {
                mean_traces[selection].at(sample - conf.sample_start) += conf.trace_values.at(traceid * trace_len + sample);
            }
        }
        auto sample_limit = std::min(mean_traces[0].size(), mean_traces[1].size());

        normalize_mean_traces(mean_traces, counter, sample_limit);

        Sample maximum_diff = -1.0;
        // do the actual DoM and save the highest peak for this keyguess
        for(size_t sample = 0; sample < sample_limit; ++sample)
        {
            Sample sample_diff = std::fabs(mean_traces[0].at(sample) - mean_traces[1].at(sample));
            dom_traces.at(keyguess).at(sample) = sample_diff;
            if(sample_diff > maximum_diff)
            {
                max_peak_per_keyguess.at(keyguess) = std::make_pair(sample_diff, keyguess);
                maximum_diff = sample_diff;
            }
        }
    }
    // sort key guesses by peak values
    std::sort(std::begin(max_peak_per_keyguess),
            std::end(max_peak_per_keyguess),
            [](std::pair<Sample,unsigned int>& fst, std::pair<Sample,unsigned int>& snd){ return fst.first > snd.first; });

    // print top 5 ranked key-guesses and their corresponding ranks
    for(unsigned int rank = 0; rank < 5; ++rank)
    {
        std::cout << std::hex << max_peak_per_keyguess.at(rank).second << ':' << max_peak_per_keyguess.at(rank).first << ", ";
    }

    conf.solved_key.at(byte) = max_peak_per_keyguess.at(0).second; // save first-ranked key guess to global structure (TODO: this is ugly...fix it!)
    std::cout << '\n';

    // write 16 best trace graphs per byte (will not be created if 'graph_out' dir does not exist, so for now, please create it yourself if you need the files!)
    std::stringstream fname;
    fname << "graph_out/combined_all_" << byte;
    std::ofstream file(fname.str(), std::ios::out);

    for(unsigned int s = 0; s < samples; ++s)
    {
        for(unsigned int rank = 0; rank < 16; ++rank)
        {
            file << dom_traces.at(max_peak_per_keyguess.at(rank).second).at(s) << ' ';
        }
        file << '\n';
    }
}

} // namespace dca

