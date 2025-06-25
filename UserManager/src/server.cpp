#include <iostream>

#include <utils/index.h>
#include "../include/server.h"


using namespace utils;
using namespace uber_backend;

Server::Server() : logger_(SingletonLogger::instance()) // initialize reference once
{
    thread_pool_ = std::make_unique<ThreadPool>(64);
    logger_.logMeta(SingletonLogger::INFO, "Server initialized", __FILE__, __LINE__, __func__);
}

void Server::initiateDatabase()
{
    auto result = thread_pool_->enqueue([](int x)
                                        { return x * x; }, 8);

    logger_.logMeta(SingletonLogger::ERROR, "Creating database instance.", __FILE__, __LINE__, __func__);

    database_ = std::make_unique<uber_database>();
    database_->initalizeDatabase();
}
