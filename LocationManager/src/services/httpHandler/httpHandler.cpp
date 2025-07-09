#include <iostream>
#include <vector>

#include <utils/index.h>

#include "../../../include/services/httpHandler/httpHandler.h"
#include "../../../include/services/httpHandler/servers/httpLocationServer.h"
#include "../../../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

HttpHandler::HttpHandler(std::shared_ptr<SharedDatabase> db)
    : SharedHttpHandler(db) {
}

void HttpHandler::createServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating a lightweight HTTP server.", __FILE__, __LINE__, __func__);
    
    auto httpLocationHandler_ = std::make_unique<UberBackend::HttpLocationServer>( "httpLocationHandler","localhost", UberUtils::CONFIG::LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT, database_);
    httpLocationHandler_->createServerMethods();

    logger_.logMeta(SingletonLogger::INFO, "Added lightweight server to vector.", __FILE__, __LINE__, __func__);
    servers_.push_back(std::move(httpLocationHandler_));
}