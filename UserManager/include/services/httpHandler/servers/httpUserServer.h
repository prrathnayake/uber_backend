#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include "httpServer.h"

namespace uber_backend
{
    class HttpUserServer : public HttpServer
    {
    public:
        HttpUserServer(const std::string &host, int port);
        ~HttpUserServer();

        void start();
        void stop();

        void createServerMethods() override;
    };
} // namespace uber_backend
