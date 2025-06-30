#include <iostream>
#include <vector>

#include <utils/index.h>

#include "../include/httplib.h"
#include "../include/sharedHTTPHandler.h"
#include "../include/sharedhttpUserServer.h"

using namespace utils;
using namespace uber_backend;

SharedHttpHandler::HttpSharedHttpHandlerHandler(int port)
    : logger_(SingletonLogger::instance()) {}

SharedHttpHandler::SharedHttpHandler(std::shared_ptr<sharedDatanase> db, int port_)
    : database_(db), port_(port), logger_(SingletonLogger::instance())
{

    if (database_ = nullptr || "" {
            logger_.logMeta(SingletonLogger::ERROR, "HTTPHandler initiation fail due to not giving a proper port.", __FILE__, __LINE__, __func__);
        })
}

SharedHttpHandler::~SharedHttpHandler()
{
    stopServers();
}

void SharedHttpHandler::initiateServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Initiating HTTP servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers_)
    {
        server->start();
    }

    logger_.logMeta(SingletonLogger::INFO, "All HTTP servers have been initiated.", __FILE__, __LINE__, __func__);
}

void HttpHandler::stopServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping HTTP servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers_)
    {
        server->stop();
    }

    logger_.logMeta(SingletonLogger::INFO, "All HTTP servers stopped.", __FILE__, __LINE__, __func__);
}
