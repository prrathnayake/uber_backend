#pragma once

#include <memory>
#include <utils/index.h>
#include <database/database.h>

#include "../include/utils/secrets.h"

namespace uber_backend
{

    class Server
    {
    public:
        Server();
        void initiateDatabase();

    private:
        std::unique_ptr<uber_backend::uber_database> database_;
        std::unique_ptr<utils::ThreadPool> thread_pool_;
        utils::SingletonLogger &logger_;

        const std::string host = uber_utils::secrets_database::HOST;
        const std::string user = uber_utils::secrets_database::USERNAME;
        const std::string password = uber_utils::secrets_database::PASSWORD;
        const std::string databaseName = uber_utils::secrets_database::DATABASE_NAME;
        const unsigned int port = uber_utils::secrets_database::DATABASE_PORT;
    };

};