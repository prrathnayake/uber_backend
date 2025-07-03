#include <iostream>
#include <vector>

#include <utils/index.h>

#include "../../../include/services/httpHandler/httpHandler.h"
#include "../../../include/services/httpHandler/servers/httpUserServer.h"
#include "../../../../sharedUtils/config.h"

using namespace utils;
using namespace UberBackend;

HttpHandler::HttpHandler(std::shared_ptr<SharedDatabase> db)
    : SharedHttpHandler(db) {
}

void HttpHandler::createServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating a lightweight HTTP server.", __FILE__, __LINE__, __func__);
    
    auto httpUserHandler_ = std::make_unique<UberBackend::HttpUserServer>( "httpUserHandler","localhost", UberUtils::CONFIG::USER_MANAGER_HTTP_USER_HANDLER_PORT, database_);
    httpUserHandler_->createServerMethods();

    logger_.logMeta(SingletonLogger::INFO, "Added lightweight server to vector.", __FILE__, __LINE__, __func__);
    servers_.push_back(std::move(httpUserHandler_));
}