#include <iostream>
#include <vector>

#include <utils/index.h>

#include "../include/sharedHTTPHandler.h"

using namespace utils;
using namespace UberBackend;

SharedHttpHandler::SharedHttpHandler(int port)
    : logger_(SingletonLogger::instance()) {}

SharedHttpHandler::SharedHttpHandler(std::shared_ptr<SharedDatabase> db)
    : database_(db), logger_(SingletonLogger::instance())
{

    if (!database_)
    {
        logger_.logMeta(SingletonLogger::ERROR, "HTTPHandler initiation fail due to not giving a proper port.", __FILE__, __LINE__, __func__);
    }
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

void SharedHttpHandler::stopServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping HTTP servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers_)
    {
        server->stop();
    }

    logger_.logMeta(SingletonLogger::INFO, "All HTTP servers stopped.", __FILE__, __LINE__, __func__);
}

bool SharedHttpHandler::servers_isEmpty()
{
    return servers_.empty();
}

void SharedHttpHandler::createServers() {
    logger_.logMeta(SingletonLogger::WARNING, "createServers() not implemented in base SharedHttpHandler.", __FILE__, __LINE__, __func__);
}
