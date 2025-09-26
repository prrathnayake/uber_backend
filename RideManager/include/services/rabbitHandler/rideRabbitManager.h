#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include <utils/index.h>

#include "../../../../sharedResources/include/sharedRabbitMQHandler.h"

namespace UberBackend
{
    class RideRabbitManager
    {
    public:
        RideRabbitManager();

        void publishDriverNotification(const std::string &driverId, const nlohmann::json &payload);

    private:
        utils::SingletonLogger &logger_;
        std::unique_ptr<SharedRabbitMQHandler> handler_;
        std::shared_ptr<SharedRabbitMQProducer> producer_;
    };
}
