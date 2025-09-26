#include "../../../include/services/kafkaHandler/rideKafkaManager.h"

#include <sstream>

#include "../../../../sharedUtils/include/config.h"

namespace UberBackend
{
    namespace
    {
        constexpr const char *kTopic = "ride_lifecycle_events";
    }

    RideKafkaManager::RideKafkaManager()
        : logger_(utils::SingletonLogger::instance())
    {
        handler_ = std::make_unique<SharedKafkaHandler>(
            UberUtils::CONFIG::getKafkaHost(),
            std::to_string(UberUtils::CONFIG::getKafkaPort()));
        producer_ = handler_->createProducer("ride_manager_producer");

        if (!producer_)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Failed to create Kafka producer for ride events",
                            __FILE__,
                            __LINE__,
                            __func__);
        }
    }

    void RideKafkaManager::publishEvent(const std::string &eventType, const nlohmann::json &payload)
    {
        if (!producer_)
        {
            return;
        }

        nlohmann::json envelope{{"type", eventType}, {"payload", payload}, {"source", "RideManager"}};
        producer_->sendMessage(kTopic, envelope.dump());
    }
}
