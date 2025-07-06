#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include <utils/index.h>

#include "../../../../sharedResources/include/sharedRouteHandler.h"
#include "../../../../sharedResources/include/sharedKafkaHandler.h"

#include "../../../include/models/user.h"
#include "../../../include/database/userDBManager.h"
#include "../../../include/services/kafkaHandler/producer/userKafkaManager.h"

namespace UberBackend
{
    class RouteHandler : public SharedRouteHandler
    {
    public:
        RouteHandler(std::shared_ptr<SharedDatabase> db);
        ~RouteHandler();

        void handleNewUser(std::shared_ptr<User> user);
        nlohmann::json handleUserLogin(std::string& username);

    private:
        std::shared_ptr<UserDBManager> userDBManager_;
        std::shared_ptr<UserKafkaManager> userKafkaManager_;
        std::shared_ptr<SharedKafkaHandler> sharedKafkaHandler_;

    };
}
