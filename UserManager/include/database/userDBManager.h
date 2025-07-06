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

        void addUserToDB(std::shared_ptr<User> user);;

        [[nodiscard]] nlohmann::json getUserByUsername(const std::string &username);
        [[nodiscard]] nlohmann::json getUserByID(int id);

    private:
        SingletonLogger &logger_;
        std::shared_ptr<SharedDatabase> database_;
    };

}