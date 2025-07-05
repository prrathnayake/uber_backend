#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <future>

#include <utils/index.h>
#include <database/database.h>

#include "httplib.h"
#include "sharedHTTPServer.h"
#include "sharedDatabase.h"

using namespace utils;
using namespace UberBackend;

namespace UberBackend
{
    class SharedHttpHandler
    {
    public:
        SharedHttpHandler(int port);
        SharedHttpHandler(std::shared_ptr<SharedDatabase> db);
        ~SharedHttpHandler();

        virtual void createServers();
        virtual void initiateServers();
        virtual void stopServers();
        virtual bool servers_isEmpty();

    protected:
        SingletonLogger &logger_;
        ThreadPool* thread_pool_;  // ✅ now using raw pointer
        std::future<void> httpServerFuture_;

        std::vector<std::unique_ptr<SharedHttpServer>> servers_;
        std::shared_ptr<SharedDatabase> database_;

        int port_;
    };
}
