#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include <utils/index.h>

#include "../../../database/database.h"

namespace uber_backend
{
    class HttpServer
    {
    public:
        HttpServer(const std::string &host, int port, std::shared_ptr<uber_backend::uber_database> db);
        ~HttpServer();

        void start();
        void stop();

        // Expose route registration
        virtual void createServerMethods() = 0;

    protected:
        utils::SingletonLogger &logger_;
        std::shared_ptr<uber_backend::uber_database> database_;

        std::string host_;
        int port_;
        std::unique_ptr<httplib::Server> server_;
        std::thread server_thread_;
        bool is_running_;
    };
} // namespace uber_backend
