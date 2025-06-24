#include <iostream>
#include "../include/server.h"

using namespace utils;
using namespace uber_backend;

Server::Server()
{
    fileLogger_ = std::make_unique<FileLogger>("log/UberBackendLog.txt");
    thread_pool_ = std::make_unique<ThreadPool>(64);
    fileLogger_->logMeta(FileLogger::INFO, "Server initialized", __FILE__, __LINE__, __func__);
}

void Server::initiateDatabase()
{
    auto result = thread_pool_->enqueue([](int x) {
        return x * x;
    }, 8);

    fileLogger_->logMeta(FileLogger::ERROR, "Creating database instance.", __FILE__, __LINE__, __func__);

    database_ = std::make_unique<uber_database>();
    database_->initalizeDatabase();  // You may need to pass logger or config
}
