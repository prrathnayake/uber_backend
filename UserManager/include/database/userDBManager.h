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

        void addUserToDB(const std::string &firstName,
                         const std::string &middleName,
                         const std::string &lastName,
                         const std::string &mobileNumber,
                         const std::string &address,
                         const std::string &email,
                         const std::string &username,
                         const std::string &password,
                         const std::string &role);

        [[nodiscard]] nlohmann::json getUserByUsername(const std::string &username);
        [[nodiscard]] nlohmann::json getUserByID(int id);

    private:
        SingletonLogger &logger_;
        std::shared_ptr<SharedDatabase> database_;
    };

}