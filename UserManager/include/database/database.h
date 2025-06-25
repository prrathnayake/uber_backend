#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/MySQLDatabase.h>
#include <utils/index.h>

namespace uber_backend {

    class uber_database {
    public:
        uber_database(const std::string &host,
                      const std::string &user,
                      const std::string &password,
                      const std::string &databaseName,
                      unsigned int port = 3306);

        ~uber_database();

        void connectDatabase();
        void disconnectDatabase();
        void runSQLScript(const std::string &relativePath);

    private:
        std::unique_ptr<database::MySQLDatabase> database;
        utils::SingletonLogger &logger_;

        std::string host;
        std::string user;
        std::string password;
        std::string databaseName;
        unsigned int port;
    };

}
