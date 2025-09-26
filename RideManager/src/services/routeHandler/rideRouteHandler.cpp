#include "../../../include/services/routeHandler/rideRouteHandler.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

#include <httplib.h>

#include "../../../../sharedUtils/include/config.h"

namespace
{
    std::string currentTimestamp()
    {
        using clock = std::chrono::system_clock;
        auto now = clock::now();
        std::time_t timeNow = clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &timeNow);
#else
        localtime_r(&timeNow, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
}

namespace UberBackend
{
    RideRouteHandler::RideRouteHandler(std::shared_ptr<SharedDatabase> db)
        : SharedRouteHandler(std::move(db)),
          logger_(utils::SingletonLogger::instance()),
          kafkaManager_(std::make_unique<RideKafkaManager>()),
          rabbitManager_(std::make_unique<RideRabbitManager>()),
          locationGateway_(std::make_unique<LocationGateway>()),
          locationClient_(UberUtils::CONFIG::getLocationManagerHost() + ":" + std::to_string(UberUtils::CONFIG::getLocationManagerGrpcPort())),
          rideSequence_(1)
    {
    }

    std::string RideRouteHandler::generateRideId()
    {
        const auto seq = rideSequence_.fetch_add(1);
        std::ostringstream oss;
        oss << "ride-" << std::setfill('0') << std::setw(8) << seq;
        return oss.str();
    }

    std::string RideRouteHandler::statusToString(RideStatus status)
    {
        switch (status)
        {
        case RideStatus::Requested:
            return "requested";
        case RideStatus::PendingDriverDecision:
            return "pending_driver";
        case RideStatus::Accepted:
            return "accepted";
        case RideStatus::InProgress:
            return "in_progress";
        case RideStatus::Completed:
            return "completed";
        case RideStatus::Cancelled:
        default:
            return "cancelled";
        }
    }

    RideRouteHandler::RideStatus RideRouteHandler::statusFromString(const std::string &value)
    {
        if (value == "requested")
        {
            return RideStatus::Requested;
        }
        if (value == "pending_driver")
        {
            return RideStatus::PendingDriverDecision;
        }
        if (value == "accepted")
        {
            return RideStatus::Accepted;
        }
        if (value == "in_progress")
        {
            return RideStatus::InProgress;
        }
        if (value == "completed")
        {
            return RideStatus::Completed;
        }
        return RideStatus::Cancelled;
    }

    nlohmann::json RideRouteHandler::rideToJson(const RideRecord &record) const
    {
        nlohmann::json payload{{"ride_id", record.id},
                               {"rider_id", record.riderId},
                               {"driver_id", record.driverId},
                               {"pickup", { {"address", record.pickupAddress}, {"lat", record.pickupLat}, {"lng", record.pickupLng} }},
                               {"dropoff", { {"address", record.dropoffAddress}, {"lat", record.dropoffLat}, {"lng", record.dropoffLng} }},
                               {"status", statusToString(record.status)},
                               {"requested_at", record.requestedAt},
                               {"updated_at", record.updatedAt}};

        if (!record.statusReason.empty())
        {
            payload["status_reason"] = record.statusReason;
        }

        return payload;
    }

    nlohmann::json RideRouteHandler::buildResponse(const std::string &status,
                                                   const std::string &message,
                                                   int httpStatus) const
    {
        return nlohmann::json{{"status", status}, {"message", message}, {"http_status", httpStatus}};
    }

    nlohmann::json RideRouteHandler::buildResponse(const RideRecord &record, int httpStatus) const
    {
        auto payload = rideToJson(record);
        payload["status"] = "success";
        payload["http_status"] = httpStatus;
        return payload;
    }

    void RideRouteHandler::setDriverAvailability(const std::string &driverId, bool available, const std::optional<std::string> &rideId)
    {
        std::scoped_lock lock(mutex_);
        auto &state = drivers_[driverId];
        state.available = available;
        if (available)
        {
            state.currentRideId.clear();
        }
        else if (rideId)
        {
            state.currentRideId = *rideId;
        }
    }

    bool RideRouteHandler::isDriverAvailable(const std::string &driverId) const
    {
        std::scoped_lock lock(mutex_);
        auto it = drivers_.find(driverId);
        return it == drivers_.end() ? false : it->second.available && it->second.currentRideId.empty();
    }

    std::optional<RideRouteHandler::RideRecord> RideRouteHandler::findRide(const std::string &rideId) const
    {
        std::scoped_lock lock(mutex_);
        auto it = rides_.find(rideId);
        if (it == rides_.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<RideRouteHandler::RideRecord> RideRouteHandler::findRidesByPredicate(const std::function<bool(const RideRecord &)> &predicate) const
    {
        std::vector<RideRecord> results;
        std::scoped_lock lock(mutex_);
        for (const auto &[id, record] : rides_)
        {
            if (predicate(record))
            {
                results.push_back(record);
            }
        }
        return results;
    }

    std::optional<nlohmann::json> RideRouteHandler::fetchDriverProfile(const std::string &driverId) const
    {
        httplib::Client client(UberUtils::CONFIG::getUserManagerHost(), static_cast<int>(UberUtils::CONFIG::getUserManagerHttpPort()));
        auto response = client.Get(("/user/" + driverId).c_str());
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

    void RideRouteHandler::persistRide(const RideRecord &record) const
    {
        if (!database_)
        {
            return;
        }

        auto toInteger = [](const std::string &value) -> long long
        {
            try
            {
                return std::stoll(value);
            }
            catch (...)
            {
                return 0;
            }
        };

        std::ostringstream query;
        query << "INSERT INTO rides(ride_identifier, user_id, driver_id, pickup_location, dropoff_location, pickup_lat, pickup_lng, dropoff_lat, dropoff_lng, requested_at, updated_at, status, status_reason) VALUES ("
              << "'" << database_->escapeString(record.id) << "',"
              << toInteger(record.riderId) << ","
              << (record.driverId.empty() ? std::string("NULL") : std::to_string(toInteger(record.driverId))) << ","
              << "'" << database_->escapeString(record.pickupAddress) << "',"
              << "'" << database_->escapeString(record.dropoffAddress) << "',"
              << record.pickupLat << ","
              << record.pickupLng << ","
              << record.dropoffLat << ","
              << record.dropoffLng << ","
              << "'" << database_->escapeString(record.requestedAt) << "',"
              << "'" << database_->escapeString(record.updatedAt) << "',"
              << "'" << database_->escapeString(statusToString(record.status)) << "',"
              << "'" << database_->escapeString(record.statusReason) << "')"
              << " ON DUPLICATE KEY UPDATE status='" << database_->escapeString(statusToString(record.status)) << "',"
              << " status_reason='" << database_->escapeString(record.statusReason) << "',"
              << " updated_at='" << database_->escapeString(record.updatedAt) << "'";

        database_->executeInsert(query.str());
    }

    nlohmann::json RideRouteHandler::handleRideRequest(const nlohmann::json &payload)
    {
        const std::vector<std::string> requiredFields{"rider_id", "pickup_location", "dropoff_location"};
        for (const auto &field : requiredFields)
        {
            if (!payload.contains(field) || payload.at(field).is_null())
            {
                return buildResponse("error", "Missing field: " + field, 400);
            }
        }

        const std::string riderId = payload.at("rider_id").get<std::string>();
        const std::string pickup = payload.at("pickup_location").get<std::string>();
        const std::string dropoff = payload.at("dropoff_location").get<std::string>();
        const double pickupLat = payload.value("pickup_lat", 0.0);
        const double pickupLng = payload.value("pickup_lng", 0.0);
        const double dropoffLat = payload.value("dropoff_lat", 0.0);
        const double dropoffLng = payload.value("dropoff_lng", 0.0);

        std::vector<std::string> nearbyDrivers;
        if (locationGateway_)
        {
            nearbyDrivers = locationGateway_->fetchNearbyDrivers(pickupLat, pickupLng);
        }

        std::string selectedDriver;
        for (const auto &candidate : nearbyDrivers)
        {
            if (isDriverAvailable(candidate))
            {
                selectedDriver = candidate;
                break;
            }
        }

        if (selectedDriver.empty())
        {
            // fallback to any available driver tracked by state
            std::scoped_lock lock(mutex_);
            for (const auto &[driverId, state] : drivers_)
            {
                if (state.available && state.currentRideId.empty())
                {
                    selectedDriver = driverId;
                    break;
                }
            }
        }

        if (selectedDriver.empty())
        {
            return buildResponse("error", "No drivers currently available", 409);
        }

        RideRecord record;
        record.id = generateRideId();
        record.riderId = riderId;
        record.driverId = selectedDriver;
        record.pickupAddress = pickup;
        record.dropoffAddress = dropoff;
        record.pickupLat = pickupLat;
        record.pickupLng = pickupLng;
        record.dropoffLat = dropoffLat;
        record.dropoffLng = dropoffLng;
        record.status = RideStatus::PendingDriverDecision;
        record.requestedAt = currentTimestamp();
        record.updatedAt = record.requestedAt;

        {
            std::scoped_lock lock(mutex_);
            rides_[record.id] = record;
        }

        setDriverAvailability(selectedDriver, false, record.id);
        persistRide(record);

        auto rideJson = rideToJson(record);
        kafkaManager_->publishEvent("ride.requested", rideJson);
        kafkaManager_->publishEvent("ride.assigned", rideJson);
        rabbitManager_->publishDriverNotification(selectedDriver, rideJson);
        locationClient_.SendLocation(riderId, pickupLat, pickupLng);

        return buildResponse(record, 201);
    }

    nlohmann::json RideRouteHandler::handleRideStatusUpdate(const std::string &rideId, const nlohmann::json &payload)
    {
        if (!payload.contains("status"))
        {
            return buildResponse("error", "Missing status field", 400);
        }

        auto rideOpt = findRide(rideId);
        if (!rideOpt)
        {
            return buildResponse("error", "Ride not found", 404);
        }

        RideRecord record = *rideOpt;
        const std::string statusValue = payload.at("status").get<std::string>();
        RideStatus newStatus = statusFromString(statusValue);
        record.updatedAt = currentTimestamp();

        switch (newStatus)
        {
        case RideStatus::Accepted:
            record.status = RideStatus::Accepted;
            setDriverAvailability(record.driverId, false, rideId);
            record.statusReason = payload.value("reason", std::string{});
            break;
        case RideStatus::InProgress:
            record.status = RideStatus::InProgress;
            record.statusReason.clear();
            break;
        case RideStatus::Completed:
            record.status = RideStatus::Completed;
            record.statusReason = payload.value("fare", std::string{});
            setDriverAvailability(record.driverId, true);
            break;
        case RideStatus::Cancelled:
            record.status = RideStatus::Cancelled;
            record.statusReason = payload.value("reason", std::string{});
            setDriverAvailability(record.driverId, true);
            break;
        case RideStatus::PendingDriverDecision:
        case RideStatus::Requested:
        default:
            record.status = newStatus;
            break;
        }

        {
            std::scoped_lock lock(mutex_);
            rides_[rideId] = record;
        }

        persistRide(record);

        auto rideJson = rideToJson(record);
        kafkaManager_->publishEvent("ride.status_changed", rideJson);
        if (!record.driverId.empty())
        {
            rabbitManager_->publishDriverNotification(record.driverId, rideJson);
        }

        return buildResponse(record);
    }

    nlohmann::json RideRouteHandler::handleDriverStatusUpdate(const std::string &driverId, const nlohmann::json &payload)
    {
        bool available = payload.value("available", false);
        std::optional<std::string> rideId;
        if (payload.contains("ride_id") && payload.at("ride_id").is_string())
        {
            rideId = payload.at("ride_id").get<std::string>();
        }

        setDriverAvailability(driverId, available, rideId);

        nlohmann::json driverState{{"driver_id", driverId}, {"available", available}};
        if (rideId)
        {
            driverState["ride_id"] = *rideId;
        }

        kafkaManager_->publishEvent("driver.status_changed", driverState);
        return nlohmann::json{{"status", "success"}, {"message", "Driver status updated"}, {"http_status", 200}, {"data", driverState}};
    }

    nlohmann::json RideRouteHandler::handleGetRide(const std::string &rideId) const
    {
        auto rideOpt = findRide(rideId);
        if (!rideOpt)
        {
            return buildResponse("error", "Ride not found", 404);
        }

        return buildResponse(*rideOpt);
    }

    nlohmann::json RideRouteHandler::handleGetRidesForUser(const std::string &userId) const
    {
        auto results = findRidesByPredicate([&userId](const RideRecord &record)
                                            { return record.riderId == userId; });

        nlohmann::json response{{"status", "success"}, {"http_status", 200}};
        response["data"] = nlohmann::json::array();
        for (const auto &record : results)
        {
            response["data"].push_back(rideToJson(record));
        }
        return response;
    }

    nlohmann::json RideRouteHandler::handleGetRidesForDriver(const std::string &driverId) const
    {
        auto results = findRidesByPredicate([&driverId](const RideRecord &record)
                                            { return record.driverId == driverId; });

        nlohmann::json response{{"status", "success"}, {"http_status", 200}};
        response["data"] = nlohmann::json::array();
        for (const auto &record : results)
        {
            response["data"].push_back(rideToJson(record));
        }
        return response;
    }

    nlohmann::json RideRouteHandler::handleGetDriverProfile(const std::string &driverId) const
    {
        auto profile = fetchDriverProfile(driverId);
        if (!profile)
        {
            return buildResponse("error", "Driver not found", 404);
        }

        nlohmann::json response{{"status", "success"}, {"http_status", 200}};
        response["data"] = *profile;
        return response;
    }
}
