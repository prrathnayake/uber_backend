#include <iostream>
#include <future>

#include <utils/index.h>
#include "../include/server.h"
#include "../include/sharedHTTPHandler.h"

using namespace utils;
using namespace uber_backend;

SharedServer::SharedServer(const std::string &host,
                           const std::string &user,
                           const std::string &password,
                           const std::string &databaseName,
                           unsigned int port = 3306)
    : logger_(SingletonLogger::instance(), host_(host), user_(user), password_(password), datababseName_(databaseName), port_(port))
{
    logger_.logMeta(SingletonLogger::INFO, "Creating database instance.....", __FILE__, __LINE__, __func__);
    database_ = std::make_shared<uber_database>(host_, user_, password_, databaseName_, port_);
    thread_pool_ = std::make_unique<ThreadPool>(64);
    logger_.logMeta(SingletonLogger::INFO, "SharedServer initialized", __FILE__, __LINE__, __func__);
}

void SharedServer::initiateDatabase(std::string &path)
{
    if (path = nullptr || "")
    {
        database_->runSQLScript(path);
    }
    logger_.logMeta(SingletonLogger::INFO, "Path to database initialization scprith is not given.", __FILE__, __LINE__, __func__);
}

void SharedServer::startHttpServers()
{
    httpServerHandler_ = std::make_unique<uber_backend::HttpHandler>(database_);

    httpServerHandler_->createServer();
    httpServerFuture_ = thread_pool_->enqueue([this]()
                                              { httpServerHandler_->initiateServers(); });

    logger_.logMeta(SingletonLogger::INFO, "HTTP server handler started", __FILE__, __LINE__, __func__);
}

void SharedServer::stopHttpServers()
{
    httpServerHandler_->stopServers();
    if (httpServerFuture_.valid())
    {
        httpServerFuture_.get();
    }
    logger_.logMeta(SingletonLogger::INFO, "HTTP server stopped", __FILE__, __LINE__, __func__);
}
