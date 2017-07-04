#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>


struct config_t
{
    int traces;
    std::vector<uint8_t> trace_values;
    std::vector<uint8_t> guess_values;
    std::array<uint8_t, 16> solved_key;
};


std::vector<uint8_t> load_file(std::string const& fname);

#endif // UTILS_H

