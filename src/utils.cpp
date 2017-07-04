#include <fstream>
#include <string>
#include <vector>
#include "utils.h"

namespace dca {

namespace utils {

// TODO documentation
// TODO error handling
std::vector<uint8_t> load_file(std::string const& fname)
{
    std::ifstream input(fname, std::ios::in | std::ios::binary);
    std::vector<uint8_t> input_values(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());

    return input_values;
}

} // namespace utils

} // namespace dca

