#pragma once

#include <memory>
#include <vector>

#include <utils/index.h>
#include <database/database.h>

#include "../../../../shared/httplib.h"
#include "./servers/httpServer.h"

namespace uber_backend
{

    class HttpHandler
    {
    public:
        HttpHandler();
        HttpHandler(std::shared_ptr<uber_backend::uber_database> db);
        ~HttpHandler();

        void createServer();    // Create and store a new HTTP server
        void initiateServers(); // Start all stored servers
        void stopServers();     // Stop all stored servers

    private:
        utils::SingletonLogger &logger_;
        std::vector<std::unique_ptr<uber_backend::HttpServer>> servers;
        std::shared_ptr<uber_backend::uber_database> database_;

    };

}
