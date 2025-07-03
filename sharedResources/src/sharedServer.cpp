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
        logger_.logMeta(SingletonLogger::ERROR, "Server initialization failed... Please enter a servername", __FILE__, __LINE__, __func__);
    }
    logger_.logMeta(SingletonLogger::INFO, "Creating database instance.....", __FILE__, __LINE__, __func__);
    database_ = std::make_shared<SharedDatabase>(host_, user_, password_, databaseName_, port_);
    thread_pool_ = std::make_unique<ThreadPool>(64);
    logger_.logMeta(SingletonLogger::INFO, "SharedServer initialized", __FILE__, __LINE__, __func__);
}

UberBackend::SharedServer::~SharedServer() = default;

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
    if (!httpServerHandler_->servers_isEmpty())
    {
        httpServerFuture_ = thread_pool_->enqueue([this]()
                                                  { httpServerHandler_->initiateServers(); });

        logger_.logMeta(SingletonLogger::INFO, "HTTP server handler started", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP servers to initialized. Please create.....", __FILE__, __LINE__, __func__);
    }
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

std::shared_ptr<SharedDatabase> SharedServer::getDatabase()
{
    logger_.logMeta(SingletonLogger::DEBUG, "SharedDatabase SharedServer::getDatabase()", __FILE__, __LINE__, __func__);
    return database_;
}
