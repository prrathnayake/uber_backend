#include <iostream>
#include "../include/sharedServer.h"

using namespace utils;
using namespace UberBackend;

SharedServer::SharedServer(
    const std::string &serverName,
    const std::string &host,
    const std::string &user,
    const std::string &password,
    const std::string &databaseName,
    unsigned int port)
    : logger_(SingletonLogger::instance()),
      serverName_(serverName),
      host_(host),
      user_(user),
      password_(password),
      databaseName_(databaseName),
      port_(port)
{
    if (serverName_.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "Server initialization failed: Server name is required", __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(SingletonLogger::INFO, "Creating database instance...", __FILE__, __LINE__, __func__);
    database_ = std::make_shared<SharedDatabase>(host_, user_, password_, databaseName_, port_);

    logger_.logMeta(SingletonLogger::INFO, "Creating thread pool with 64 threads...", __FILE__, __LINE__, __func__);
    thread_pool_ = std::shared_ptr<ThreadPool>(&ThreadPool::instance(64), [](ThreadPool *) {});

    logger_.logMeta(SingletonLogger::INFO, "Creating route handler.", __FILE__, __LINE__, __func__);
    sharedRouteHandler_ = std::make_unique<SharedRouteHandler>(database_);

    logger_.logMeta(SingletonLogger::INFO, "SharedServer initialized", __FILE__, __LINE__, __func__);
}

SharedServer::~SharedServer() = default;

void SharedServer::initiateDatabase(const std::string &path)
{
    if (!path.empty())
    {
        database_->runSQLScript(path);
        logger_.logMeta(SingletonLogger::INFO, "Database initialized from script", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::INFO, "No database initialization script path provided.", __FILE__, __LINE__, __func__);
    }
}

void SharedServer::startHttpServers()
{
    if (!httpServerHandler_ || httpServerHandler_->servers_isEmpty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP servers to initialize. Please create them first.", __FILE__, __LINE__, __func__);
        return;
    }

    httpServerHandler_->initiateServers();

    logger_.logMeta(SingletonLogger::INFO, "HTTP server handler started", __FILE__, __LINE__, __func__);
}

void SharedServer::stopHttpServers()
{
    httpServerHandler_->stopServers();
    logger_.logMeta(SingletonLogger::INFO, "HTTP server stopped", __FILE__, __LINE__, __func__);
}

std::shared_ptr<SharedDatabase> SharedServer::getDatabase()
{
    logger_.logMeta(SingletonLogger::DEBUG, "SharedDatabase SharedServer::getDatabase() called", __FILE__, __LINE__, __func__);
    return database_;
}
