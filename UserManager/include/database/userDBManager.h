#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include <utils/index.h>

#include "../models/user.h"
#include "database.h"

namespace UberBackend
{

    class UserDBManager
    {
    public:
        UserDBManager(std::shared_ptr<SharedDatabase> db);
        ~UserDBManager();

        void addUserToDB(std::shared_ptr<User> user);

        [[nodiscard]] nlohmann::json getUserByUsername(const std::string &username);
        [[nodiscard]] nlohmann::json getUserByID(int id);
        [[nodiscard]] nlohmann::json getAllUsers();

        bool updateUserById(int id, const nlohmann::json &data);
        bool updateUserPassword(int id, const std::string &newPasswordHash);
        bool updateUserFields(int id, const nlohmann::json &fields);
        bool deleteUserById(int id);

        [[nodiscard]] nlohmann::json searchUsersByUsername(const std::string &username);

    private:
        SingletonLogger &logger_;
        std::shared_ptr<SharedDatabase> database_;
    };

}
