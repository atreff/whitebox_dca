#ifndef UTILS_H
#define UTILS_H

#include <array>
#include <string>
#include <vector>

namespace dpa {

struct config_t
{
    int traces;
    std::vector<uint8_t> trace_values;
    std::vector<uint8_t> guess_values;
    std::array<uint8_t, 16> solved_key;
};

namespace utils {

std::vector<uint8_t> load_file(std::string const& fname);

} // namespace utils

} // namespace dpa

#endif // UTILS_H

