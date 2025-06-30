#pragma once

#include <iostreaam>
#include <string>
#include <memory>
#include <future>
#include <utils/index.h>
#include <database/database.h>

#include "sharedHttpHandler.h"

using namespace utils :

    namespace uber_backend
{
    class SharedServer
    {
    public:
        sharedServer(const std::string &host,
                     const std::string &user,
                     const std::string &password,
                     const std::string &databaseName,
                     unsigned int port = 3306);
        ~sharedServer();

        virtual void initiateDatabase() = 0;
        virtual void startHttpServers() = 0;
        virtual void stopHttpServers() = 0;

    protected:
        SingletonLogger &logger_;
        std::shared_ptr<uber_backend::SharedDatabase> database_;
        std::unique_ptr<ThreadPool> thread_pool_;
        std::unique_ptr<uber_backend::SharedHttpHandler> httpServerHandler_;
        std::future<void> httpServerFuture_;

        const std::string host_;
        const std::string user_;
        const std::string password_;
        const std::string databaseName_;
        const unsigned int port_;
    };
}
