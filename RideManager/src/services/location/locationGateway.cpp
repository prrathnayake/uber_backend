#include "../../../include/services/location/locationGateway.h"

#include <sstream>

#include <httplib.h>

#include "../../../../sharedUtils/include/config.h"

namespace UberBackend
{
    LocationGateway::LocationGateway()
    {
        host_ = UberUtils::CONFIG::getLocationManagerHost();
        port_ = static_cast<int>(UberUtils::CONFIG::getLocationManagerHttpPort());
    }

    std::vector<std::string> LocationGateway::fetchNearbyDrivers(double latitude, double longitude, double radiusKm) const
    {
        httplib::Client client(host_, port_);
        std::stringstream path;
        path << "/location/nearby?lat=" << latitude << "&lng=" << longitude << "&radius=" << radiusKm << "&role=driver";

        auto response = client.Get(path.str().c_str());
        if (!response || response->status >= 400)
        {
            return {};
        }

        nlohmann::json payload = nlohmann::json::parse(response->body, nullptr, false);
        if (payload.is_discarded())
        {
            return {};
        }

        std::vector<std::string> drivers;
        if (payload.contains("data") && payload.at("data").is_array())
        {
            for (const auto &entry : payload.at("data"))
            {
                if (entry.contains("user_id"))
                {
                    drivers.push_back(entry.at("user_id").get<std::string>());
                }
            }
        }
        else if (payload.contains("users") && payload.at("users").is_array())
        {
            for (const auto &entry : payload.at("users"))
            {
                if (entry.contains("user_id"))
                {
                    drivers.push_back(entry.at("user_id").get<std::string>());
                }
            }
        }

        return drivers;
    }

    std::optional<nlohmann::json> LocationGateway::fetchLocationForUser(const std::string &userId) const
    {
        httplib::Client client(host_, port_);
        auto response = client.Get(("/location/" + userId).c_str());
        if (!response || response->status >= 400)
        {
            return std::nullopt;
        }

        nlohmann::json payload = nlohmann::json::parse(response->body, nullptr, false);
        if (payload.is_discarded())
        {
            return std::nullopt;
        }

        return payload;
    }
}
