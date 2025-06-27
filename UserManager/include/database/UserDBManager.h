#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/MySQLDatabase.h>
#include <utils/index.h>

#include "../models/user.h"
#include "database.h"

namespace uber_backend
{

    class UserDBManager
    {
    public:
        UserDBManager(td::shared_ptr<uber_backend::uber_database> db);
        ~UserDBManager();

        void addUserToDB(const std::string &firstName,
                         const std::string &middleName,
                         const std::string &lastName,
                         const std::string &mobileNumber,
                         const std::string &address,
                         const std::string &email,
                         const std::string &username,
                         const std::string &password);
        uber_backend::User getUserByID();

    private:
        std::shared_ptr<uber_backend::uber_database> database_;
    };

}
