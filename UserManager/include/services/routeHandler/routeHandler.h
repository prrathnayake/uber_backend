#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include <utils/index.h>
#include <algorithms/sha256/index.h>

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
        nlohmann::json handleUserLogin(std::string &username);

        // Extended CRUD operations
        nlohmann::json handleGetUserById(const std::string &userId);
        bool handleUpdateUser(const std::string &userId, const nlohmann::json &data);
        bool handleDeleteUser(const std::string &userId);
        nlohmann::json handleGetAllUsers();

        // PATCH operations
        bool handlePasswordUpdate(const std::string &userId, const std::string &oldPwd, const std::string &newPwd);
        bool handlePartialProfileUpdate(const std::string &userId, const nlohmann::json &updates);

        // Search
        nlohmann::json searchUsersByUsername(const std::string &username);

    private:
        std::shared_ptr<UserDBManager> userDBManager_;
        std::shared_ptr<UserKafkaManager> userKafkaManager_;
        std::shared_ptr<SharedKafkaHandler> kafkaHandler_;
    };
}
