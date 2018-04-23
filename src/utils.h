#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <string>
#include <vector>

namespace dca {

struct config_t
{
    unsigned int traces;
    unsigned int samples_per_trace;
    unsigned int sample_start;
    unsigned int sample_end;
    std::vector<uint8_t> trace_values;
    std::vector<uint8_t> guess_values;
    std::array<uint8_t, 16> solved_key;
};

namespace utils {

std::vector<uint8_t> load_file(std::string const& fname);

void log_error(std::string const &line);

} // namespace utils

} // namespace dca

#endif // UTILS_H

