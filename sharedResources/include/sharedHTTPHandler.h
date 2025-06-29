#pragma once

#include <memory>
#include <vector>

#include <utils/index.h>
#include <database/database.h>

#include "httplib.h"
#include "sharedHttpServer.h"

using namespace utils;

namespace uber_backend
{

    class SharedHttpHandler
    {
    public:
        SharedHttpHandler(int port);
        SharedHttpHandler(std::shared_ptr<uber_backend::sharedDatanase> db, int port_);
        ~SharedHttpHandler();


        virtual void createServer() = 0;
        virtual void initiateServers();     
        virtual void stopServers();          

    protected:
        SingletonLogger &logger_;
        std::vector<std::unique_ptr<uber_backend::SharedHttpServer>> servers_;
        std::shared_ptr<uber_backend::sharedDatabase> database_;

        int port_;
    };

}
