#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include "httpServer.h"

#include "../../../database/userDBManager.h"

namespace uber_backend
{
    class HttpUserServer : public HttpServer
    {
    public:
        HttpUserServer(const std::string &host, int port, std::shared_ptr<uber_database> db);
        ~HttpUserServer();

        void start();
        void stop();

        void createServerMethods() override;

    private:
        std::shared_ptr<uber_backend::UserDBManager> userDBManager_;
    };
} // namespace uber_backend
