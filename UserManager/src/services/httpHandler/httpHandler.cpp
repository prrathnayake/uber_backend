#include <iostream>
#include <vector>

#include <utils/index.h>

#include "../../../../sharedResources/include/httplib.h"
#include "../../../include/services/httpHandler/httpHandler.h"
#include "../../../include/services/httpHandler/servers/httpUserServer.h"
#include "../../../include/utils/config.h"

using namespace utils;
using namespace uber_backend;

HttpHandler::HttpHandler()
    : logger_(SingletonLogger::instance()) {}

HttpHandler::HttpHandler(std::shared_ptr<uber_database> db)
    : database_(db), logger_(SingletonLogger::instance()) {}


HttpHandler::~HttpHandler()
{
    // Optionally ensure all servers stop when the service is destroyed
    stopServers();
}

void HttpHandler::createServer()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating a lightweight HTTP server.", __FILE__, __LINE__, __func__);
    
    auto httpUserHandler = std::make_unique<uber_backend::HttpUserServer>("localhost", uber_utils::CONFIG::HTTP_USER_HANDLER_PORT, database_);
    httpUserHandler->createServerMethods();

    logger_.logMeta(SingletonLogger::INFO, "Added lightweight server to vector.", __FILE__, __LINE__, __func__);
    servers.push_back(std::move(httpUserHandler));
}

void HttpHandler::initiateServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Initiating servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers)
    {
        server->start();
    }

    logger_.logMeta(SingletonLogger::INFO, "All servers have been initiated.", __FILE__, __LINE__, __func__);
}

void HttpHandler::stopServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers)
    {
        server->stop();
    }

    logger_.logMeta(SingletonLogger::INFO, "All servers stopped.", __FILE__, __LINE__, __func__);
}
