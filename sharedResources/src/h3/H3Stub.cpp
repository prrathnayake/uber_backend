#include "../../include/h3/index.h"

#include <cmath>

namespace h3 {

H3Index H3::getH3Index(double lat, double lon, int resolution) const
{
    const std::uint64_t latBits = static_cast<std::uint64_t>((lat + 90.0) * 1000000.0);
    const std::uint64_t lonBits = static_cast<std::uint64_t>((lon + 180.0) * 1000000.0);
    const std::uint64_t resBits = static_cast<std::uint64_t>(resolution & 0xF);
    return (latBits << 24) ^ (lonBits << 8) ^ resBits;
}

std::string H3::toString(H3Index index) const
{
    return std::to_string(index);
}

H3Index H3::fromString(const std::string &value) const
{
    try
    {
        return static_cast<H3Index>(std::stoull(value));
    }
    catch (...)
    {
        return 0;
    }
}

bool H3::isValid(H3Index index) const
{
    return index != 0;
}

std::vector<H3Index> H3::getNeighbors(H3Index origin, int kRing) const
{
    std::vector<H3Index> neighbors;
    const int count = std::max(0, kRing);
    neighbors.reserve(static_cast<std::size_t>(count));
    for (int i = 1; i <= count; ++i)
    {
        neighbors.push_back(origin + static_cast<H3Index>(i));
    }
    return neighbors;
}

} // namespace h3

