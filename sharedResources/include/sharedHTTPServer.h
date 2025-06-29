#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include <utils/index.h>


#include "sharedDatabase.h"

using namespace utils;

namespace uber_backend
{
    class SharedHttpServer
    {
    public:
        SharedHttpServer(const std::string &serverName, const std::string &host, int port, std::shared_ptr<uber_backend::SharedDatabase> db);
        ~SharedHttpServer();

        virtual void start();
        virtual void stop();

        virtual void createServerMethods() = 0;

    protected:
        SingletonLogger &logger_;
        std::shared_ptr<uber_backend::SharedDatabase> database_;

        std::string serverName_;
        std::string host_;
        int port_;
        std::unique_ptr<httplib::Server> server_;
        std::thread server_thread_;
        bool is_running_;
    };
}
