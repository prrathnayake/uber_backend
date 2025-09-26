#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace UberBackend
{
    class LocationGateway
    {
    public:
        LocationGateway();

        std::vector<std::string> fetchNearbyDrivers(double latitude, double longitude, double radiusKm = 5.0) const;
        std::optional<nlohmann::json> fetchLocationForUser(const std::string &userId) const;

    private:
        std::string host_;
        int port_;
    };
}
