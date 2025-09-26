#include "../../../include/services/routeHandler/rideRouteHandler.h"

#include <chrono>
#include <cctype>
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

        if (record.fareRecorded)
        {
            payload["fare"] = {{"amount", record.fareAmount}, {"currency", record.currency}};
            payload["payment"] = {{"method", record.paymentMethod},
                                    {"status", record.paymentRecorded ? "completed" : "pending"}};
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

        std::string fareValue = "NULL";
        if (record.fareRecorded)
        {
            std::ostringstream fareStream;
            fareStream << std::fixed << std::setprecision(2) << record.fareAmount;
            fareValue = fareStream.str();
        }

        const auto rideIdEscaped = database_->escapeString(record.id);
        const auto pickupEscaped = database_->escapeString(record.pickupAddress);
        const auto dropoffEscaped = database_->escapeString(record.dropoffAddress);
        const auto requestedAtEscaped = database_->escapeString(record.requestedAt);
        const auto updatedAtEscaped = database_->escapeString(record.updatedAt);
        const auto statusEscaped = database_->escapeString(statusToString(record.status));
        const auto reasonEscaped = database_->escapeString(record.statusReason);

        std::ostringstream query;
        query << "INSERT INTO rides(ride_identifier, user_id, driver_id, pickup_location, dropoff_location, pickup_lat, pickup_lng, dropoff_lat, dropoff_lng, requested_at, updated_at, status, status_reason, fare) VALUES ("
              << "'" << rideIdEscaped << "',"
              << toInteger(record.riderId) << ","
              << (record.driverId.empty() ? std::string("NULL") : std::to_string(toInteger(record.driverId))) << ","
              << "'" << pickupEscaped << "',"
              << "'" << dropoffEscaped << "',"
              << record.pickupLat << ","
              << record.pickupLng << ","
              << record.dropoffLat << ","
              << record.dropoffLng << ","
              << "'" << requestedAtEscaped << "',"
              << "'" << updatedAtEscaped << "',"
              << "'" << statusEscaped << "',"
              << "'" << reasonEscaped << "',"
              << fareValue << ")"
              << " ON DUPLICATE KEY UPDATE status='" << statusEscaped << "',"
              << " status_reason='" << reasonEscaped << "',"
              << " updated_at='" << updatedAtEscaped << "',"
              << " fare=" << fareValue;

        database_->executeInsert(query.str());
    }

    std::optional<double> RideRouteHandler::parseFareAmount(const nlohmann::json &payload) const
    {
        if (!payload.contains("fare"))
        {
            return std::nullopt;
        }

        const auto &fareNode = payload.at("fare");
        double amount = 0.0;

        if (fareNode.is_number_float() || fareNode.is_number_integer())
        {
            amount = fareNode.get<double>();
        }
        else if (fareNode.is_string())
        {
            const auto raw = fareNode.get<std::string>();
            std::string normalized;
            bool decimalEncountered = false;
            for (char ch : raw)
            {
                if (std::isdigit(static_cast<unsigned char>(ch)))
                {
                    normalized.push_back(ch);
                }
                else if ((ch == '.' || ch == ',') && !decimalEncountered)
                {
                    normalized.push_back('.');
                    decimalEncountered = true;
                }
            }

            if (normalized.empty())
            {
                return std::nullopt;
            }

            try
            {
                amount = std::stod(normalized);
            }
            catch (...)
            {
                return std::nullopt;
            }
        }
        else
        {
            return std::nullopt;
        }

        if (amount < 0.0)
        {
            return std::nullopt;
        }

        return amount;
    }

    void RideRouteHandler::creditDriverWallet(const std::string &driverId, double amount, const RideRecord &record)
    {
        if (driverId.empty() || amount <= 0.0)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Skipping wallet credit because of missing driver or non-positive fare", __FILE__, __LINE__, __func__);
            return;
        }

        {
            std::scoped_lock lock(mutex_);
            auto &state = drivers_[driverId];
            state.walletBalance += amount;
            state.lifetimeEarnings += amount;
        }

        std::ostringstream oss;
        oss << "Credited driver " << driverId << " wallet with " << std::fixed << std::setprecision(2) << amount
            << " for ride " << record.id;
        logger_.logMeta(utils::SingletonLogger::INFO, oss.str(), __FILE__, __LINE__, __func__);
    }

    double RideRouteHandler::getDriverWalletBalance(const std::string &driverId) const
    {
        std::scoped_lock lock(mutex_);
        auto it = drivers_.find(driverId);
        return it == drivers_.end() ? 0.0 : it->second.walletBalance;
    }

    bool RideRouteHandler::recordPaymentInDatabase(const RideRecord &record, double amount, const std::string &method)
    {
        if (!database_)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Cannot persist payment without an active database connection", __FILE__, __LINE__, __func__);
            return false;
        }

        auto ridePrimaryKey = lookupRidePrimaryKey(record.id);
        if (!ridePrimaryKey)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Ride primary key lookup failed while recording payment for " + record.id,
                            __FILE__,
                            __LINE__,
                            __func__);
            return false;
        }

        std::ostringstream amountStream;
        amountStream << std::fixed << std::setprecision(2) << amount;

        std::string methodToPersist = method.empty() ? std::string{"wallet"} : method;
        const auto escapedMethod = database_->escapeString(methodToPersist);

        std::ostringstream query;
        query << "INSERT INTO payments (ride_id, amount, method, status) VALUES ("
              << *ridePrimaryKey << ", " << amountStream.str() << ", '" << escapedMethod
              << "', 'completed') ON DUPLICATE KEY UPDATE amount=VALUES(amount), method=VALUES(method), status='completed'";

        if (!database_->executeInsert(query.str()))
        {
            logger_.logMeta(utils::SingletonLogger::ERROR,
                            "Failed to persist payment record for ride " + record.id,
                            __FILE__,
                            __LINE__,
                            __func__);
            return false;
        }

        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Recorded payment of " + amountStream.str() + " for ride " + record.id + " using method " + methodToPersist,
                        __FILE__,
                        __LINE__,
                        __func__);
        return true;
    }

    std::optional<long long> RideRouteHandler::lookupRidePrimaryKey(const std::string &rideIdentifier) const
    {
        if (!database_)
        {
            return std::nullopt;
        }

        const auto escapedIdentifier = database_->escapeString(rideIdentifier);
        std::ostringstream query;
        query << "SELECT id FROM rides WHERE ride_identifier='" << escapedIdentifier << "' LIMIT 1";

        auto rows = database_->fetchRows(query.str());
        if (rows.empty())
        {
            return std::nullopt;
        }

        const auto &row = rows.front();
        auto it = row.find("id");
        if (it == row.end())
        {
            return std::nullopt;
        }

        try
        {
            return std::stoll(it->second);
        }
        catch (...)
        {
            logger_.logMeta(utils::SingletonLogger::ERROR,
                            "Failed to parse ride primary key from database response for ride " + rideIdentifier,
                            __FILE__,
                            __LINE__,
                            __func__);
            return std::nullopt;
        }
    }

    RideRouteHandler::DriverState RideRouteHandler::snapshotDriverState(const std::string &driverId) const
    {
        std::scoped_lock lock(mutex_);
        auto it = drivers_.find(driverId);
        if (it == drivers_.end())
        {
            return {};
        }
        return it->second;
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
        const std::string currency = payload.value("currency", std::string{"USD"});
        const std::string paymentMethod = payload.value("payment_method", std::string{"wallet"});

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
        record.currency = currency;
        record.paymentMethod = paymentMethod;

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
            record.statusReason = payload.value("reason", record.statusReason);
            record.currency = payload.value("currency", record.currency);
            record.paymentMethod = payload.value("payment_method", record.paymentMethod);
            if (record.fareRecorded)
            {
                logger_.logMeta(utils::SingletonLogger::WARNING,
                                "Ride completion received but fare already recorded; skipping duplicate settlement for ride " + rideId,
                                __FILE__,
                                __LINE__,
                                __func__);
            }
            else
            {
                auto fareAmount = parseFareAmount(payload);
                if (fareAmount)
                {
                    record.fareAmount = *fareAmount;
                    record.fareRecorded = true;
                    if (!record.driverId.empty())
                    {
                        creditDriverWallet(record.driverId, record.fareAmount, record);
                    }
                    record.paymentRecorded = recordPaymentInDatabase(record, record.fareAmount, record.paymentMethod);
                    if (!record.paymentRecorded)
                    {
                        logger_.logMeta(utils::SingletonLogger::WARNING,
                                        "Ride payment persistence failed; wallet credit was performed in-memory", __FILE__, __LINE__, __func__);
                    }
                }
                else
                {
                    logger_.logMeta(utils::SingletonLogger::WARNING,
                                    "Ride completion missing fare amount; skipping wallet credit for ride " + rideId,
                                    __FILE__,
                                    __LINE__,
                                    __func__);
                }
            }
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

        const auto stateSnapshot = snapshotDriverState(driverId);

        nlohmann::json driverState{{"driver_id", driverId},
                                   {"available", available},
                                   {"wallet_balance", stateSnapshot.walletBalance},
                                   {"lifetime_earnings", stateSnapshot.lifetimeEarnings}};
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
        const auto stateSnapshot = snapshotDriverState(driverId);
        response["wallet"] = {{"balance", stateSnapshot.walletBalance},
                               {"lifetime_earnings", stateSnapshot.lifetimeEarnings}};
        return response;
    }
}
