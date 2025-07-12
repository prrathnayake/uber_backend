#pragma once

#include <memory>
#include <string>

#include <utils/index.h>
#include "../../../models/user.h"
#include "../../../../../sharedResources/include/sharedKafkaConsumer.h"

namespace UberBackend
{

    class UserKafkaManager
    {
    public:
        explicit UserKafkaManager(std::shared_ptr<SharedKafkaConsumer> kafkaConsumer);
        ~UserKafkaManager();

        std::shared_ptr<User> consumeNewUser();


    private:
        SingletonLogger &logger_;
        std::shared_ptr<SharedKafkaConsumer> kafkaConsumer_;
        const std::string topic_ = "user-events";  // default topic
    };

}
