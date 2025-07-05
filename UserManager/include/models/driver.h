#pragma once

#include <string>
#include <iostream>

#include <utils/index.h>

#include "paymentDetails.h"
#include "bankDetails.h"
#include "vehicleDetails.h"
#include "user.h"

using namespace utils;

namespace UberBackend
{
    class Driver : public User
    {
    public:
        Driver();

        Driver(const std::string &firstName,
               const std::string &middleName,
               const std::string &lastName,
               const std::string &countryCode,
               const std::string &mobileNumber,
               const std::string &address,
               const std::string &email,
               const std::string &username,
               const std::string &passwordHash,
               const std::string &preferredLanguage = "en",
               const std::string &currency = "USD",
               const std::string &country = "");

        ~Driver();

    private:
        VehicleDetails vehicleDetails_;
    };
}
