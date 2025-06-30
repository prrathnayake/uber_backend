#pragma once

#include <string>
#include <iostream>

#include <utils/index.h>

#include "paymentDetails.h"
#include "bankDetails.h"

using namespace utils;

namespace uber_backend
{
    class Rider
    {
    public:
        Rider();

        Rider(const std::string &firstName,
             const std::string &middleName,
             const std::string &lastName,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash,
             const std::string &role); 

        ~Rider();

    private:
        SingletonLogger &logger_;

        std::string role_ = "driver";
    };
}
