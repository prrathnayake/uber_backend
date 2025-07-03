#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <database/MySQLDatabase.h>
#include <utils/index.h>

using namespace database;
using namespace utils;

namespace UberBackend
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

        virtual void connectDatabase();
        virtual void disconnectDatabase();
        virtual void runSQLScript(const std::string &relativePath);
        virtual void executeInsert(const std::string &query);
        virtual void executeUpdate(const std::string &query);
        virtual void executeDelete(const std::string &query);
        virtual void executeSelect(const std::string &query);
        virtual std::string escapeString(const std::string &input);

    protected:
        std::shared_ptr<MySQLDatabase> database_;
        SingletonLogger &logger_;

        std::string databaseName_;
        std::string host_;
        std::string user_;
        std::string password_;
        unsigned int port_;
    };

}
