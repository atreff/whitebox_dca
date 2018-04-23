#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "utils.h"

namespace dca {

namespace utils {


static bool enable_logging = true;
static std::string error_prefix = "[ERROR]:";

void log_error(std::string const &line)
{
    if(dca::utils::enable_logging)
    {
        std::cerr << error_prefix << line << std::endl;
    }
}

std::vector<uint8_t> load_file(std::string const& fname)
{
    std::ifstream input;
    // please throw an exception on failure
    input.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        input.open(fname, std::ios::in | std::ios::binary);
    } catch(std::system_error &e)
    {
        std::stringstream ss;
        // use strerror() to get a useful error message
        ss << "Error opening " << fname << ": " << strerror(errno);
        dca::utils::log_error(ss.str());
        throw;
    }

    // read file into vector using C++ iterator magic ;)
    std::vector<uint8_t> input_values(
            (std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());

    return input_values;
}

} // namespace utils

} // namespace dca

