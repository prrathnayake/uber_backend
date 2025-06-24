#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/index.h>
#include <utils/index.h>
#include "../utils/secrets.h"

namespace uber_backend
{
    class uber_database
    {
    public:
        uber_database();
        ~uber_database();

        void initalizeDatabase();
        void addDriver(std::string);

    private:
        std::unique_ptr<database::MySQLDatabase> database;

        const std::string host = uber_backend::uber_utils::secrets_database::HOST;
        const std::string user = uber_backend::uber_utils::secrets_database::USERNAME;
        const std::string password = uber_backend::uber_utils::secrets_database::PASSWORD;
        const std::string databaseName = uber_backend::uber_utils::secrets_database::DATABASE_NAME;
        unsigned int port = uber_backend::uber_utils::secrets_database::DATABASE_PORT;
    };
}
