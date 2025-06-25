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
            void Get(const std::string &path, httplib::Server::Handler handler);
            void Post(const std::string &path, httplib::Server::Handler handler);

        private:
            std::string host_;
            int port_;
            std::unique_ptr<httplib::Server> server_;
            std::thread server_thread_;
            bool is_running_;
        };
} // namespace uber_backend
