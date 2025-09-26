#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../../../sharedResources/include/sharedRouteHandler.h"

namespace UberBackend
{
    /**
     * @brief HTTP route coordinator for the LocationManager service.
     *
     * The handler translates HTTP level payloads into concrete database
     * operations while keeping all validation and formatting logic in a
     * single place.  This keeps the HTTP server thin and ensures that the
     * same routines can be re-used from other transports (gRPC, Kafka
     * consumers, etc.).
     */
    class RouteHandler : public SharedRouteHandler
    {
    public:
        explicit RouteHandler(std::shared_ptr<SharedDatabase> db);

        [[nodiscard]] nlohmann::json handleLocationUpdate(const nlohmann::json &payload);
        [[nodiscard]] nlohmann::json handleGetLocationByUserId(const std::string &userId);
        [[nodiscard]] nlohmann::json handleFindNearby(double latitude,
                                                      double longitude,
                                                      const std::string &role,
                                                      double radiusKm = 5.0);
        [[nodiscard]] nlohmann::json handleDeleteLocation(const std::string &userId);

    private:
        [[nodiscard]] std::optional<nlohmann::json> fetchLocationRecord(long long userId);
        [[nodiscard]] bool ensureDatabase();

        static double parseDouble(const std::string &value, double fallback = 0.0);
        static long long parseInteger(const std::string &value, long long fallback = 0);
        static bool isValidLatitude(double value);
        static bool isValidLongitude(double value);
        static double haversine(double lat1, double lon1, double lat2, double lon2);
    };
}
