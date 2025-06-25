#include <iostream>
#include <future>  // For std::future

#include <utils/index.h>
#include "../include/server.h"
#include "../include/services/httpService/httpService.h"

using namespace utils;
using namespace uber_backend;

Server::Server()
    : logger_(SingletonLogger::instance())
{
    thread_pool_ = std::make_unique<ThreadPool>(64);  // initialize thread pool with 64 threads
    httpServerHandler_ = std::make_unique<uber_backend::HttpService>();

    logger_.logMeta(SingletonLogger::INFO, "Server initialized", __FILE__, __LINE__, __func__);
}

void Server::initiateDatabase()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating database instance.", __FILE__, __LINE__, __func__);

    database_ = std::make_unique<uber_database>(host, user, password, databaseName, port);

    database_->runSQLScript("../../sql_scripts/database_init.sql");
}

void Server::startHttpServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Starting HTTP Servers.", __FILE__, __LINE__, __func__);

    httpServerHandler_->createServer();

    // Launch the HTTP server asynchronously on the thread pool, store the future
    httpServerFuture_ = thread_pool_->enqueue([this]() {
        httpServerHandler_->initiateServers();
    });

    logger_.logMeta(SingletonLogger::INFO, "HTTP server started", __FILE__, __LINE__, __func__);
}

void Server::stopHttpServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping HTTP Servers.", __FILE__, __LINE__, __func__);

    // Stop all servers
    httpServerHandler_->stopServers();

    // Wait for the async server task to finish if valid
    if (httpServerFuture_.valid())
    {
        httpServerFuture_.get();
    }

    logger_.logMeta(SingletonLogger::INFO, "HTTP server stopped", __FILE__, __LINE__, __func__);
}
