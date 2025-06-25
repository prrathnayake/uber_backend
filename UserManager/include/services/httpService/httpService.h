#pragma once

#include <memory>
#include <vector>
#include <httplib.h>

#include <utils/index.h>
#include <database/database.h>
#include "../include/utils/secrets.h"
#include "./httpServer.h"

namespace uber_backend
{

    class HttpService
    {
    public:
        HttpService();
        ~HttpService();

        void createServer();    // Create and store a new HTTP server
        void initiateServers(); // Start all stored servers
        void stopServers();     // Stop all stored servers

    private:
        utils::SingletonLogger &logger_;
        std::vector<std::unique_ptr<uber_backend::HttpServer>> servers;
    };

}
