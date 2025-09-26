#include <iostream>
#include <vector>

#include <utils/index.h>

#include "services/httpHandler/httpHandler.h"
#include "services/httpHandler/servers/httpLocationServer.h"
#include "config.h"

using namespace utils;
using namespace UberBackend;

HttpHandler::HttpHandler(std::shared_ptr<SharedDatabase> db)
    : SharedHttpHandler(std::move(db))
{
}

void HttpHandler::createServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating a lightweight HTTP server.", __FILE__, __LINE__, __func__);

    auto locationServer = std::make_unique<HttpLocationServer>(
        "httpLocationHandler",
        "localhost",
        UberUtils::CONFIG::LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT,
        database_);
    locationServer->createServerMethods();

    logger_.logMeta(SingletonLogger::INFO, "Added lightweight server to vector.", __FILE__, __LINE__, __func__);
    servers_.push_back(std::move(locationServer));
}
