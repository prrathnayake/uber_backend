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

    // create a database instance
    logger_.logMeta(SingletonLogger::INFO, "Creating database instance...", __FILE__, __LINE__, __func__);
    database_ = std::make_shared<SharedDatabase>(host_, user_, password_, databaseName_, port_);

    // Thread pool for handling concurrent tasks in the server
    logger_.logMeta(SingletonLogger::INFO, "Creating thread pool", __FILE__, __LINE__, __func__);
    thread_pool_ = std::shared_ptr<ThreadPool>(&ThreadPool::instance(64), [](ThreadPool *) {});

    // Create Router Handler for handling HTTP routes
    logger_.logMeta(SingletonLogger::INFO, "Creating route handler.", __FILE__, __LINE__, __func__);
    sharedRouteHandler_ = std::make_unique<SharedRouteHandler>(database_);

    logger_.logMeta(SingletonLogger::INFO, "SharedServer initialized Successfully", __FILE__, __LINE__, __func__);
}

SharedServer::~SharedServer()
{
    stopConsumers();
    stopGrpcServer();
    if (httpServerHandler_)
    {
        stopHttpServers();
    }
}

// This function runs a SQL scriptse
void SharedServer::runScript(const std::string &path)
{
    if (!path.empty())
    {
        logger_.logMeta(SingletonLogger::DEBUG, "run : database_->runSQLScript(path);", __FILE__, __LINE__, __func__);

        database_->runSQLScript(path);
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "No database initialization script path provided.", __FILE__, __LINE__, __func__);
    }
}

void SharedServer::initiateDatabase(const std::string &path)
{
    runScript(path);
    logger_.logMeta(SingletonLogger::INFO, "Database initialized from script", __FILE__, __LINE__, __func__);
}

void SharedServer::distoryDatabase(const std::string &path)
{
    runScript(path);
    logger_.logMeta(SingletonLogger::INFO, "Database distory from script", __FILE__, __LINE__, __func__);
}

void SharedServer::startHttpServers()
{
    // check if the HTTP server handler is created and has servers to start
    if (!httpServerHandler_ || httpServerHandler_->servers_isEmpty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP servers to initialize. Please create them first.", __FILE__, __LINE__, __func__);
        return;
    }
    else
    {
        httpServerHandler_->initiateServers();
        logger_.logMeta(SingletonLogger::INFO, "HTTP server handler started", __FILE__, __LINE__, __func__);
    }
}

void SharedServer::stopHttpServers()
{
    if (!httpServerHandler_)
    {
        return;
    }

    httpServerHandler_->stopServers();
    logger_.logMeta(SingletonLogger::INFO, "HTTP server stopped", __FILE__, __LINE__, __func__);
}

void SharedServer::ensureGrpcServer(const std::string &address)
{
    if (address.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "gRPC server address cannot be empty", __FILE__, __LINE__, __func__);
        return;
    }

    if (!grpcServer_)
    {
        grpcServer_ = std::make_unique<SharedgPRCServer>(address);
        logger_.logMeta(SingletonLogger::INFO, "gRPC server configured for address " + address, __FILE__, __LINE__, __func__);
    }
}

void SharedServer::startGrpcServer()
{
    if (!grpcServer_)
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "Cannot start gRPC server before calling ensureGrpcServer",
                        __FILE__,
                        __LINE__,
                        __func__);
        return;
    }

    if (grpcServer_->isRunning())
    {
        logger_.logMeta(SingletonLogger::WARNING, "gRPC server is already running", __FILE__, __LINE__, __func__);
        return;
    }

    auto *server = grpcServer_.get();
    grpcFuture_ = thread_pool_->enqueue([server]()
                                        { server->Run(); });
    logger_.logMeta(SingletonLogger::INFO, "gRPC server startup dispatched", __FILE__, __LINE__, __func__);
}

void SharedServer::stopGrpcServer()
{
    if (!grpcServer_)
    {
        return;
    }

    grpcServer_->Shutdown();

    if (grpcFuture_.valid())
    {
        grpcFuture_.get();
    }

    grpcServer_.reset();
}

std::shared_ptr<SharedDatabase> SharedServer::getDatabase()
{
    logger_.logMeta(SingletonLogger::DEBUG, "SharedDatabase SharedServer::getDatabase() called", __FILE__, __LINE__, __func__);
    return database_;
}

void SharedServer::stopConsumers()
{
    if (sharedKafkaHandler_)
    {
        sharedKafkaHandler_->stopConsumers();
    }

    if (sharedRabbitHandler_)
    {
        sharedRabbitHandler_->stopConsumers();
    }
}
