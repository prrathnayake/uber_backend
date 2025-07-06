#pragma once

#include <memory>
#include <string>

#include <utils/index.h>
#include "../../../models/user.h"
#include "../../../../../sharedResources/include/sharedKafkaProducer.h"

namespace UberBackend
{

    class UserKafkaManager
    {
    public:
        UserKafkaManager();
        ~UserKafkaManager();

        void produceNewUser(std::shared_ptr<User> user);

    private:
        SingletonLogger &logger_;
        std::shared_ptr<SharedKafkaProducer> kafkaProducer_;
        std::string topic_;
    };

}
