#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

namespace uber_backend
{
    class HttpServer
    {
    public:
        HttpServer(const std::string &host, int port);
        ~HttpServer();

        void start();
        void stop();

        // Expose route registration
        virtual void createServerMethods() = 0;

    protected:
        std::string host_;
        int port_;
        std::unique_ptr<httplib::Server> server_;
        std::thread server_thread_;
        bool is_running_;
    };
} // namespace uber_backend
