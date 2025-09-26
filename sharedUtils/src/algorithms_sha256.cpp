#include "../include/algorithms/sha256/index.h"

#include <functional>
#include <iomanip>
#include <sstream>

namespace algorithms {

std::string toBinary(const std::string &input)
{
    std::string binary;
    binary.reserve(input.size() * 8);
    for (unsigned char ch : input)
    {
        for (int bit = 7; bit >= 0; --bit)
        {
            binary.push_back((ch & (1u << bit)) ? '1' : '0');
        }
    }
    return binary;
}

std::string hashComputation(const std::string &binaryData)
{
    const std::string salted1 = binaryData + "::seed1";
    const std::string salted2 = binaryData + "::seed2";
    const std::string salted3 = binaryData + "::seed3";
    const std::string salted4 = binaryData + "::seed4";

    std::size_t h1 = std::hash<std::string>{}(salted1);
    std::size_t h2 = std::hash<std::string>{}(salted2);
    std::size_t h3 = std::hash<std::string>{}(salted3);
    std::size_t h4 = std::hash<std::string>{}(salted4);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (auto value : {h1, h2, h3, h4})
    {
        oss << std::setw(sizeof(std::size_t) * 2) << value;
    }
    return oss.str();
}

} // namespace algorithms

