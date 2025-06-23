#pragma once
#include <iostream>

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

    private:
        database::MySQLDatabase *database;
        utils::FileLogger *fileLogger = new utils::FileLogger();

        const std::string host = uber_backend::uber_utils::secrets_database::HOST;
        const std::string user = uber_backend::uber_utils::secrets_database::USERNAME;
        const std::string password = uber_backend::uber_utils::secrets_database::PASSWORD;
        const std::string databaseName = uber_backend::uber_utils::secrets_database::DATABASE_NAME;
        unsigned int port = uber_backend::uber_utils::secrets_database::DATABASE_PORT;
    };
}