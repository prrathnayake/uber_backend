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
    kafkaHandler_ = std::make_shared<SharedKafkaHandler>(host, std::to_string(port));
    logger_.logMeta(SingletonLogger::DEBUG, "Implementing Kafka producers", __FILE__, __LINE__, __func__);

    userKafkaManager_ = std::make_shared<UserKafkaManager>(kafkaHandler_->createProducer("userKafkaManager_"));

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

// ✅ GET /user/:id
nlohmann::json RouteHandler::handleGetUserById(const std::string &userId)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleGetUserById().", __FILE__, __LINE__, __func__);
    return userDBManager_->getUserByID(std::stoi(userId));
}

// ✅ PUT /user/:id
bool RouteHandler::handleUpdateUser(const std::string &userId, const nlohmann::json &data)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleUpdateUser().", __FILE__, __LINE__, __func__);
    return userDBManager_->updateUserById(std::stoi(userId), data);
}

// ✅ DELETE /user/:id
bool RouteHandler::handleDeleteUser(const std::string &userId)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleDeleteUser().", __FILE__, __LINE__, __func__);
    return userDBManager_->deleteUserById(std::stoi(userId));
}

// ✅ GET /users
nlohmann::json RouteHandler::handleGetAllUsers()
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleGetAllUsers().", __FILE__, __LINE__, __func__);
    return userDBManager_->getAllUsers();
}

// ✅ PATCH /user/:id/password
bool RouteHandler::handlePasswordUpdate(const std::string &userId, const std::string &oldPwd, const std::string &newPwd)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handlePasswordUpdate().", __FILE__, __LINE__, __func__);
    auto user = userDBManager_->getUserByID(std::stoi(userId));

    std::string storedHash = user["password_hash"];
    std::string oldHash = algorithms::hashComputation(algorithms::toBinary(oldPwd));

    if (storedHash == oldHash)
    {
        std::string newHash = algorithms::hashComputation(algorithms::toBinary(newPwd));
        return userDBManager_->updateUserPassword(std::stoi(userId), newHash);
    }

    return false;
}

// ✅ PATCH /user/:id/profile
bool RouteHandler::handlePartialProfileUpdate(const std::string &userId, const nlohmann::json &updates)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handlePartialProfileUpdate().", __FILE__, __LINE__, __func__);
    return userDBManager_->updateUserFields(std::stoi(userId), updates);
}

// ✅ GET /user/search?username=
nlohmann::json RouteHandler::searchUsersByUsername(const std::string &username)
{
    logger_.logMeta(SingletonLogger::DEBUG, "searchUsersByUsername().", __FILE__, __LINE__, __func__);
    return userDBManager_->searchUsersByUsername(username);
}
