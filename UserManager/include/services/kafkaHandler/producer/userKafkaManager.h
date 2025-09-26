#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include <utils/index.h>
#include "../../../models/user.h"
#include "../../../../../sharedResources/include/sharedKafkaProducer.h"

namespace UberBackend
{

    class UserKafkaManager
    {
    public:
        explicit UserKafkaManager(std::shared_ptr<SharedKafkaProducer> kafkaProducer);
        ~UserKafkaManager();

        void produceNewUser(std::shared_ptr<User> user);
        void produceUpdatedUser(std::shared_ptr<User> user);
        void produceDeletedUser(const std::string &userId);

    private:
        void sendEvent(const std::string &eventType, const nlohmann::json &data);

        SingletonLogger &logger_;
        std::shared_ptr<SharedKafkaProducer> kafkaProducer_;
        const std::string topic_ = "user-events";  // default topic
    };

}
