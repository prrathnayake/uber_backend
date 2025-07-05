#include "../../include/models/driver.h"

namespace UberBackend
{
    Driver::Driver()
        : User() {}

    Driver::Driver(const std::string &firstName,
                   const std::string &middleName,
                   const std::string &lastName,
                   const std::string &countryCode,
                   const std::string &mobileNumber,
                   const std::string &address,
                   const std::string &email,
                   const std::string &username,
                   const std::string &passwordHash,
                   const std::string &preferredLanguage,
                   const std::string &currency,
                   const std::string &country)
        : User(firstName, middleName, lastName, countryCode, mobileNumber, address, email, username, passwordHash, "driver", preferredLanguage, currency, country)
    {
    }

    Driver::~Driver() {}

}
