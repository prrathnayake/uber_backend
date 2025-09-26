#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../../../sharedResources/include/sharedRouteHandler.h"
#include "../../../../sharedResources/include/sharedgRPCClient.h"
#include "../kafkaHandler/rideKafkaManager.h"
#include "../location/locationGateway.h"
#include "../rabbitHandler/rideRabbitManager.h"

namespace UberBackend
{
    class RideRouteHandler : public SharedRouteHandler
    {
    public:
        explicit RideRouteHandler(std::shared_ptr<SharedDatabase> db);

        [[nodiscard]] nlohmann::json handleRideRequest(const nlohmann::json &payload);
        [[nodiscard]] nlohmann::json handleRideStatusUpdate(const std::string &rideId, const nlohmann::json &payload);
        [[nodiscard]] nlohmann::json handleDriverStatusUpdate(const std::string &driverId, const nlohmann::json &payload);
        [[nodiscard]] nlohmann::json handleGetRide(const std::string &rideId) const;
        [[nodiscard]] nlohmann::json handleGetRidesForUser(const std::string &userId) const;
        [[nodiscard]] nlohmann::json handleGetRidesForDriver(const std::string &driverId) const;
        [[nodiscard]] nlohmann::json handleGetDriverProfile(const std::string &driverId) const;

    private:
        enum class RideStatus
        {
            Requested,
            PendingDriverDecision,
            Accepted,
            InProgress,
            Completed,
            Cancelled
        };

        struct RideRecord
        {
            std::string id;
            std::string riderId;
            std::string driverId;
            std::string pickupAddress;
            std::string dropoffAddress;
            double pickupLat{0.0};
            double pickupLng{0.0};
            double dropoffLat{0.0};
            double dropoffLng{0.0};
            RideStatus status{RideStatus::Requested};
            std::string statusReason;
            std::string requestedAt;
            std::string updatedAt;
        };

        struct DriverState
        {
            bool available{false};
            std::string currentRideId;
        };

        [[nodiscard]] std::string generateRideId();
        [[nodiscard]] static std::string statusToString(RideStatus status);
        [[nodiscard]] static RideStatus statusFromString(const std::string &value);
        [[nodiscard]] nlohmann::json rideToJson(const RideRecord &record) const;
        [[nodiscard]] nlohmann::json buildResponse(const std::string &status,
                                                   const std::string &message,
                                                   int httpStatus = 200) const;
        [[nodiscard]] nlohmann::json buildResponse(const RideRecord &record, int httpStatus = 200) const;

        void setDriverAvailability(const std::string &driverId, bool available, const std::optional<std::string> &rideId = std::nullopt);
        [[nodiscard]] bool isDriverAvailable(const std::string &driverId) const;
        [[nodiscard]] std::optional<RideRecord> findRide(const std::string &rideId) const;
        [[nodiscard]] std::vector<RideRecord> findRidesByPredicate(const std::function<bool(const RideRecord &)> &predicate) const;
        [[nodiscard]] std::optional<nlohmann::json> fetchDriverProfile(const std::string &driverId) const;
        void persistRide(const RideRecord &record) const;

        utils::SingletonLogger &logger_;
        std::unique_ptr<RideKafkaManager> kafkaManager_;
        std::unique_ptr<RideRabbitManager> rabbitManager_;
        std::unique_ptr<LocationGateway> locationGateway_;
        LocationClient locationClient_;

        mutable std::mutex mutex_;
        std::unordered_map<std::string, RideRecord> rides_;
        std::unordered_map<std::string, DriverState> drivers_;
        std::atomic<std::uint64_t> rideSequence_;
    };
}
