#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/MySQLDatabase.h>
#include <utils/index.h>

using namespace database;
using namespace utils;

namespace uber_backend
{

    class SharedDatabase
    {
    public:
        SharedDatabase(const std::string &host,
                       const std::string &user,
                       const std::string &password,
                       const std::string &databaseName,
                       unsigned int port = 3306);

        virtual ~SharedDatabase();

        virtual void connectDatabase() = 0;
        virtual void disconnectDatabase() = 0;
        virtual void runSQLScript(const std::string &relativePath) = 0;
        virtual void executeInsert(const std::string &query) = 0;
        virtual void executeUpdate(const std::string &query) = 0;
        virtual void executeDelete(const std::string &query) = 0;
        virtual void executeSelect(const std::string &query) = 0;
        virtual std::string escapeString(const std::string &input) = 0;

    protected:
        std::shared_ptr<MySQLDatabase> database_;
        SingletonLogger &logger_;

        std::string host_;
        std::string user_;
        std::string password_;
        std::string databaseName_;
        unsigned int port_;
    };

}
