#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace h3 {

using H3Index = std::uint64_t;

class H3 {
public:
    H3() = default;

    H3Index getH3Index(double lat, double lon, int resolution) const;
    std::string toString(H3Index index) const;
    H3Index fromString(const std::string &value) const;
    bool isValid(H3Index index) const;
    std::vector<H3Index> getNeighbors(H3Index origin, int kRing) const;
};

} // namespace h3

