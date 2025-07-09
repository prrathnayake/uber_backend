#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/MySQLDatabase.h>
#include <utils/index.h>

#include "../../sharedResources/include/sharedDatabase.h"

namespace UberBackend
{
    class uber_database : public SharedDatabase
    {
    public:
        uber_database(const std::string &host,
                      const std::string &user,
                      const std::string &password,
                      const std::string &databaseName,
                      unsigned int port = 3306);
    };

}
