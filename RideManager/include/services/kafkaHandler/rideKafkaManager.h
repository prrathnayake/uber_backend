#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include <utils/index.h>

#include "../../../../sharedResources/include/sharedKafkaHandler.h"

namespace UberBackend
{
    class RideKafkaManager
    {
    public:
        RideKafkaManager();

        void publishEvent(const std::string &eventType, const nlohmann::json &payload);

    private:
        utils::SingletonLogger &logger_;
        std::unique_ptr<SharedKafkaHandler> handler_;
        std::shared_ptr<SharedKafkaProducer> producer_;
    };
}
