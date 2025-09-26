#include "../../../include/services/rabbitHandler/rideRabbitManager.h"

#include "../../../../sharedUtils/include/config.h"

namespace UberBackend
{
    namespace
    {
        constexpr const char *kQueueName = "driver_notifications";
    }

    RideRabbitManager::RideRabbitManager()
        : logger_(utils::SingletonLogger::instance())
    {
        SharedRabbitMQHandler::ConnectionOptions options{
            UberUtils::CONFIG::getRabbitMQHost(),
            std::to_string(UberUtils::CONFIG::getRabbitMQPort()),
            UberUtils::CONFIG::getRabbitMQUsername(),
            UberUtils::CONFIG::getRabbitMQPassword(),
            UberUtils::CONFIG::getRabbitMQVHost()};

        handler_ = std::make_unique<SharedRabbitMQHandler>(options);
        producer_ = handler_->createProducer("ride_manager_driver_notifications", kQueueName);

        if (!producer_)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Failed to create RabbitMQ producer for driver notifications",
                            __FILE__,
                            __LINE__,
                            __func__);
        }
    }

    void RideRabbitManager::publishDriverNotification(const std::string &driverId, const nlohmann::json &payload)
    {
        if (!producer_)
        {
            return;
        }

        nlohmann::json envelope{{"driver_id", driverId}, {"payload", payload}};
        producer_->publish(envelope.dump());
    }
}
