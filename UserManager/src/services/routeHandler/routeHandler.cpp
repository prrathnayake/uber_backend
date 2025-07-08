#include <iostream>
#include <filesystem>

#include "../../../include/services/routeHandler/routeHandler.h"
#include "../../../../sharedUtils/include/config.h"

using namespace UberBackend;

RouteHandler::RouteHandler(std::shared_ptr<SharedDatabase> db)
    : SharedRouteHandler(db)
{
    userDBManager_ = std::make_shared<UserDBManager>(database_);
    logger_.logMeta(SingletonLogger::INFO, "Creating Kafka handler...", __FILE__, __LINE__, __func__);
    std::string host = UberUtils::CONFIG::KAFKA_HOST;
    int port = UberUtils::CONFIG::KAFKA_PORT;
    sharedKafkaHandler_ = std::make_shared<SharedKafkaHandler>(host, std::to_string(port));
    logger_.logMeta(SingletonLogger::DEBUG, "Implementing Kafka producers", __FILE__, __LINE__, __func__);

    userKafkaManager_ = std::make_shared<UserKafkaManager>();

    logger_.logMeta(SingletonLogger::INFO, "RouteHandler constructed.", __FILE__, __LINE__, __func__);
}

RouteHandler::~RouteHandler() {}

void RouteHandler::handleNewUser(std::shared_ptr<User> user)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleNewUser().", __FILE__, __LINE__, __func__);
    userDBManager_->addUserToDB(user);
    userKafkaManager_->produceNewUser(user);
}

nlohmann::json RouteHandler::handleUserLogin(std::string &username)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleUserLogin().", __FILE__, __LINE__, __func__);
    return userDBManager_->getUserByUsername(username);
}
