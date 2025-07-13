#include <iostream>

#include "../include/server.h"

using namespace utils;
using namespace UberBackend;

Server::Server(const std::string &serverName,
               const std::string &host,
               const std::string &user,
               const std::string &password,
               const std::string &dbName,
               unsigned int port)
    : SharedServer(serverName, host, user, password, dbName, port)
{
}

void Server::createHttpServers()
{
    logger_.logMeta(SingletonLogger::INFO, "create HTTP Server Handler.", __FILE__, __LINE__, __func__);

    // implementing the HttpHandler to handle HTTP requests and passing the database details
    httpServerHandler_ = std::make_unique<HttpHandler>(database_);
    logger_.logMeta(SingletonLogger::DEBUG, "run : httpServerHandler_->createServers();", __FILE__, __LINE__, __func__);
    httpServerHandler_->createServers();
}

void Server::startConsumers()
{
}