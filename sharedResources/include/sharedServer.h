#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <future>

#include <utils/index.h>
#include <database/database.h>

#include "sharedHTTPHandler.h"
#include "sharedDatabase.h"

using namespace utils;

namespace UberBackend
{
    class SharedServer
    {
    public:
        SharedServer(const std::string &serverName,
                     const std::string &host,
                     const std::string &user,
                     const std::string &password,
                     const std::string &databaseName,
                     unsigned int port = 3306);
        ~SharedServer();

        virtual void initiateDatabase(const std::string &path);
        virtual void createHttpServers() = 0;
        virtual void startHttpServers();
        virtual void stopHttpServers();
        virtual std::shared_ptr<SharedDatabase> getDatabase();

    protected:
        SingletonLogger &logger_;
        std::shared_ptr<SharedDatabase> database_;
        std::unique_ptr<ThreadPool> thread_pool_;
        std::unique_ptr<SharedHttpHandler> httpServerHandler_;
        std::future<void> httpServerFuture_;

        const std::string serverName_;
        const std::string host_;
        const std::string user_;
        const std::string password_;
        const std::string databaseName_;
        const unsigned int port_;
    };
}
