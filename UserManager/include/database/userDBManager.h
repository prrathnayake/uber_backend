#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <utils/index.h>

#include "../models/user/user.h"
#include "database.h"

namespace uber_backend
{

    class UserDBManager
    {
    public:
        UserDBManager(std::shared_ptr<uber_backend::uber_database> db);
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

        void getUserByID();

    private:
        utils::SingletonLogger &logger_;
        std::shared_ptr<uber_backend::uber_database> database_;
    };

}
