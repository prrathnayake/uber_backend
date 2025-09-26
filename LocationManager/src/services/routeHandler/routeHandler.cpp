#include "../../../include/services/routeHandler/routehandler.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

using nlohmann::json;

namespace
{
    constexpr double kEarthRadiusKm = 6371.0;
    constexpr double kDefaultRadiusKm = 5.0;
}

namespace UberBackend
{
    RouteHandler::RouteHandler(std::shared_ptr<SharedDatabase> db)
        : SharedRouteHandler(std::move(db))
    {
    }

    json RouteHandler::handleLocationUpdate(const json &payload)
    {
        json response{{"status", "error"}, {"http_status", 500}};
        logger_.logMeta(SingletonLogger::DEBUG,
                        "Processing location update payload",
                        __FILE__,
                        __LINE__,
                        __func__);

        if (!ensureDatabase())
        {
            response["message"] = "Database connection is not initialised";
            return response;
        }

        if (!payload.is_object())
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Location update payload must be a JSON object",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Payload must be a JSON object";
            response["http_status"] = 400;
            return response;
        }

        long long userId = -1;
        if (payload.contains("user_id"))
        {
            try
            {
                userId = payload.at("user_id").get<long long>();
            }
            catch (const json::exception &)
            {
                logger_.logMeta(SingletonLogger::ERROR,
                                "Field 'user_id' must be numeric",
                                __FILE__,
                                __LINE__,
                                __func__);
                response["message"] = "Field 'user_id' must be a number";
                response["http_status"] = 400;
                return response;
            }
        }
        else if (payload.contains("rider_id"))
        {
            try
            {
                userId = payload.at("rider_id").get<long long>();
            }
            catch (const json::exception &)
            {
                logger_.logMeta(SingletonLogger::ERROR,
                                "Field 'rider_id' must be numeric",
                                __FILE__,
                                __LINE__,
                                __func__);
                response["message"] = "Field 'rider_id' must be a number";
                response["http_status"] = 400;
                return response;
            }
        }

        if (userId <= 0)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Location update requires a positive user id",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "A positive user identifier is required";
            response["http_status"] = 400;
            return response;
        }

        if (!payload.contains("latitude") || !payload.contains("longitude"))
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Latitude/longitude missing in location update",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Latitude and longitude are required";
            response["http_status"] = 400;
            return response;
        }

        double latitude = std::numeric_limits<double>::quiet_NaN();
        double longitude = std::numeric_limits<double>::quiet_NaN();
        try
        {
            latitude = payload.at("latitude").get<double>();
            longitude = payload.at("longitude").get<double>();
        }
        catch (const json::exception &)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Latitude or longitude is not numeric",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Latitude and longitude must be numeric";
            response["http_status"] = 400;
            return response;
        }

        if (!isValidLatitude(latitude) || !isValidLongitude(longitude))
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Latitude or longitude outside accepted range",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Latitude or longitude is out of range";
            response["http_status"] = 400;
            return response;
        }

        std::optional<std::string> optionalName;
        if (payload.contains("name") && !payload.at("name").is_null())
        {
            try
            {
                optionalName = payload.at("name").get<std::string>();
            }
            catch (const json::exception &)
            {
                logger_.logMeta(SingletonLogger::ERROR,
                                "Field 'name' must be a string",
                                __FILE__,
                                __LINE__,
                                __func__);
                response["message"] = "Field 'name' must be a string";
                response["http_status"] = 400;
                return response;
            }
        }

        std::string escapedName = "NULL";
        if (optionalName && !optionalName->empty())
        {
            escapedName = "'" + database_->escapeString(*optionalName) + "'";
        }

        std::ostringstream query;
        query.setf(std::ios::fixed);
        query << std::setprecision(6);

        auto existingRecord = fetchLocationRecord(userId);
        const bool wasExisting = existingRecord.has_value();
        bool upsertSuccess = false;

        if (existingRecord)
        {
            query << "UPDATE locations SET latitude = " << latitude
                  << ", longitude = " << longitude;
            if (optionalName)
            {
                query << ", name = " << escapedName;
            }
            query << " WHERE rider_id = " << userId;
            upsertSuccess = database_->executeUpdate(query.str());
        }
        else
        {
            query << "INSERT INTO locations (rider_id, latitude, longitude, name) VALUES ("
                  << userId << ", " << latitude << ", " << longitude << ", " << escapedName << ")";
            upsertSuccess = database_->executeInsert(query.str());
        }

        if (!upsertSuccess)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Failed to persist location information",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Failed to persist location information";
            return response;
        }

        auto latestRecord = fetchLocationRecord(userId);
        if (!latestRecord)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Location saved but could not be reloaded",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Location saved but could not be retrieved";
            return response;
        }

        logger_.logMeta(SingletonLogger::INFO,
                        std::string(wasExisting ? "Updated" : "Created") + " location for user " + std::to_string(userId),
                        __FILE__,
                        __LINE__,
                        __func__);

        response["status"] = "success";
        response["http_status"] = wasExisting ? 200 : 201;
        response["location"] = *latestRecord;
        response["message"] = wasExisting ? "Location updated" : "Location created";
        return response;
    }

    json RouteHandler::handleGetLocationByUserId(const std::string &userId)
    {
        json response{{"status", "error"}, {"http_status", 500}};
        logger_.logMeta(SingletonLogger::DEBUG,
                        "Fetching location by user id",
                        __FILE__,
                        __LINE__,
                        __func__);

        if (!ensureDatabase())
        {
            response["message"] = "Database connection is not initialised";
            return response;
        }

        long long id = -1;
        try
        {
            id = std::stoll(userId);
        }
        catch (const std::exception &)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Location lookup received a non-numeric identifier",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "User identifier must be numeric";
            response["http_status"] = 400;
            return response;
        }

        if (id <= 0)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Location lookup requires a positive identifier",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "User identifier must be positive";
            response["http_status"] = 400;
            return response;
        }

        auto record = fetchLocationRecord(id);
        if (!record)
        {
            logger_.logMeta(SingletonLogger::WARNING,
                            "No stored location for user " + std::to_string(id),
                            __FILE__,
                            __LINE__,
                            __func__);
            response["status"] = "not_found";
            response["http_status"] = 404;
            response["message"] = "No location found for the requested user";
            return response;
        }

        logger_.logMeta(SingletonLogger::INFO,
                        "Returned stored location for user " + std::to_string(id),
                        __FILE__,
                        __LINE__,
                        __func__);
        response["status"] = "success";
        response["http_status"] = 200;
        response["location"] = *record;
        return response;
    }

    json RouteHandler::handleFindNearby(double latitude,
                                        double longitude,
                                        const std::string &role,
                                        double radiusKm)
    {
        json response{{"status", "error"}, {"http_status", 500}};
        logger_.logMeta(SingletonLogger::DEBUG,
                        "Processing nearby location lookup",
                        __FILE__,
                        __LINE__,
                        __func__);

        if (!ensureDatabase())
        {
            response["message"] = "Database connection is not initialised";
            return response;
        }

        if (!isValidLatitude(latitude) || !isValidLongitude(longitude))
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Nearby search received invalid coordinates",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Latitude or longitude is out of range";
            response["http_status"] = 400;
            return response;
        }

        if (!std::isfinite(radiusKm) || radiusKm <= 0)
        {
            logger_.logMeta(SingletonLogger::WARNING,
                            "Nearby search radius invalid; falling back to default",
                            __FILE__,
                            __LINE__,
                            __func__);
            radiusKm = kDefaultRadiusKm;
        }

        const double latRadians = latitude * M_PI / 180.0;
        const double cosLat = std::cos(latRadians);
        const double latDelta = radiusKm / 111.0; // Roughly 111 km per degree latitude
        const double lngDelta = radiusKm / (111.0 * std::max(std::abs(cosLat), 1e-6));

        std::ostringstream query;
        query.setf(std::ios::fixed);
        query << std::setprecision(6)
              << "SELECT rider_id, latitude, longitude, name FROM locations WHERE latitude BETWEEN "
              << (latitude - latDelta) << " AND " << (latitude + latDelta)
              << " AND longitude BETWEEN " << (longitude - lngDelta) << " AND " << (longitude + lngDelta);

        auto rows = database_->fetchRows(query.str());

        std::vector<std::pair<double, json>> matches;
        matches.reserve(rows.size());

        for (const auto &row : rows)
        {
            if (!row.count("latitude") || !row.count("longitude") || !row.count("rider_id"))
            {
                continue;
            }

            const double rowLat = parseDouble(row.at("latitude"), std::numeric_limits<double>::quiet_NaN());
            const double rowLng = parseDouble(row.at("longitude"), std::numeric_limits<double>::quiet_NaN());

            if (!isValidLatitude(rowLat) || !isValidLongitude(rowLng))
            {
                continue;
            }

            const double distance = haversine(latitude, longitude, rowLat, rowLng);
            if (distance > radiusKm)
            {
                continue;
            }

            json entry = {
                {"user_id", parseInteger(row.at("rider_id"), 0)},
                {"latitude", rowLat},
                {"longitude", rowLng},
                {"distance_km", distance}
            };

            if (row.count("name") && !row.at("name").empty())
            {
                entry["name"] = row.at("name");
            }

            matches.emplace_back(distance, std::move(entry));
        }

        std::sort(matches.begin(), matches.end(),
                  [](const auto &lhs, const auto &rhs)
                  { return lhs.first < rhs.first; });

        json nearby = json::array();
        for (auto &match : matches)
        {
            nearby.push_back(std::move(match.second));
        }

        logger_.logMeta(SingletonLogger::INFO,
                        "Nearby search returned " + std::to_string(nearby.size()) + " result(s)",
                        __FILE__,
                        __LINE__,
                        __func__);

        response["status"] = "success";
        response["http_status"] = 200;
        response["role"] = role;
        response["radius_km"] = radiusKm;
        response["nearby"] = std::move(nearby);
        return response;
    }

    json RouteHandler::handleDeleteLocation(const std::string &userId)
    {
        json response{{"status", "error"}, {"http_status", 500}};
        logger_.logMeta(SingletonLogger::DEBUG,
                        "Deleting stored location",
                        __FILE__,
                        __LINE__,
                        __func__);

        if (!ensureDatabase())
        {
            response["message"] = "Database connection is not initialised";
            return response;
        }

        long long id = -1;
        try
        {
            id = std::stoll(userId);
        }
        catch (const std::exception &)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Delete location received a non-numeric identifier",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "User identifier must be numeric";
            response["http_status"] = 400;
            return response;
        }

        if (id <= 0)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Delete location requires a positive identifier",
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "User identifier must be positive";
            response["http_status"] = 400;
            return response;
        }

        auto existingRecord = fetchLocationRecord(id);
        if (!existingRecord)
        {
            logger_.logMeta(SingletonLogger::WARNING,
                            "Attempted to delete non-existent location for user " + std::to_string(id),
                            __FILE__,
                            __LINE__,
                            __func__);
            response["status"] = "not_found";
            response["http_status"] = 404;
            response["message"] = "No location found for the requested user";
            return response;
        }

        std::ostringstream query;
        query << "DELETE FROM locations WHERE rider_id = " << id;
        if (!database_->executeDelete(query.str()))
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Failed to delete stored location for user " + std::to_string(id),
                            __FILE__,
                            __LINE__,
                            __func__);
            response["message"] = "Failed to delete location";
            return response;
        }

        logger_.logMeta(SingletonLogger::INFO,
                        "Deleted stored location for user " + std::to_string(id),
                        __FILE__,
                        __LINE__,
                        __func__);
        response["status"] = "success";
        response["http_status"] = 200;
        response["message"] = "Location deleted";
        return response;
    }

    std::optional<json> RouteHandler::fetchLocationRecord(long long userId)
    {
        if (!database_ || userId <= 0)
        {
            return std::nullopt;
        }

        std::ostringstream query;
        query.setf(std::ios::fixed);
        query << "SELECT rider_id, latitude, longitude, name FROM locations WHERE rider_id = "
              << userId << " LIMIT 1";

        auto rows = database_->fetchRows(query.str());
        if (rows.empty())
        {
            return std::nullopt;
        }

        const auto &row = rows.front();
        if (!row.count("rider_id") || !row.count("latitude") || !row.count("longitude"))
        {
            return std::nullopt;
        }

        json result = {
            {"user_id", parseInteger(row.at("rider_id"), 0)},
            {"latitude", parseDouble(row.at("latitude"), 0.0)},
            {"longitude", parseDouble(row.at("longitude"), 0.0)}
        };

        if (row.count("name") && !row.at("name").empty())
        {
            result["name"] = row.at("name");
        }

        return result;
    }

    bool RouteHandler::ensureDatabase()
    {
        if (database_)
        {
            return true;
        }

        logger_.logMeta(SingletonLogger::ERROR,
                        "RouteHandler attempted to use an uninitialised database",
                        __FILE__,
                        __LINE__,
                        __func__);
        return false;
    }

    double RouteHandler::parseDouble(const std::string &value, double fallback)
    {
        try
        {
            if (value.empty())
            {
                return fallback;
            }
            return std::stod(value);
        }
        catch (const std::exception &)
        {
            return fallback;
        }
    }

    long long RouteHandler::parseInteger(const std::string &value, long long fallback)
    {
        try
        {
            if (value.empty())
            {
                return fallback;
            }
            return std::stoll(value);
        }
        catch (const std::exception &)
        {
            return fallback;
        }
    }

    bool RouteHandler::isValidLatitude(double value)
    {
        return std::isfinite(value) && value >= -90.0 && value <= 90.0;
    }

    bool RouteHandler::isValidLongitude(double value)
    {
        return std::isfinite(value) && value >= -180.0 && value <= 180.0;
    }

    double RouteHandler::haversine(double lat1, double lon1, double lat2, double lon2)
    {
        const double latRad1 = lat1 * M_PI / 180.0;
        const double latRad2 = lat2 * M_PI / 180.0;
        const double deltaLat = (lat2 - lat1) * M_PI / 180.0;
        const double deltaLon = (lon2 - lon1) * M_PI / 180.0;

        const double a = std::pow(std::sin(deltaLat / 2.0), 2) +
                         std::cos(latRad1) * std::cos(latRad2) *
                             std::pow(std::sin(deltaLon / 2.0), 2);
        const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        return kEarthRadiusKm * c;
    }
}
