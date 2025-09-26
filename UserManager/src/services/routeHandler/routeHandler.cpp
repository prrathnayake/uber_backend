#include <chrono>
#include <filesystem>
#include <iostream>

#include "../../../include/services/routeHandler/routeHandler.h"
#include "../../../../sharedUtils/include/config.h"

using namespace UberBackend;

RouteHandler::RouteHandler(std::shared_ptr<SharedDatabase> db)
    : SharedRouteHandler(db)
{
    userDBManager_ = std::make_shared<UserDBManager>(database_);
    logger_.logMeta(SingletonLogger::INFO, "Creating Kafka handler...", __FILE__, __LINE__, __func__);
    std::string host = UberUtils::CONFIG::getKafkaHost();
    unsigned int port = UberUtils::CONFIG::getKafkaPort();
    kafkaHandler_ = std::make_shared<SharedKafkaHandler>(host, std::to_string(port));
    logger_.logMeta(SingletonLogger::DEBUG, "Implementing Kafka producers", __FILE__, __LINE__, __func__);

    auto producer = kafkaHandler_->createProducer("userKafkaManager_");
    if (producer)
    {
        userKafkaManager_ = std::make_shared<UserKafkaManager>(producer);
    }
    else
    {
        logger_.logMeta(SingletonLogger::WARNING, "Failed to initialise Kafka producer for user events", __FILE__, __LINE__, __func__);
    }

    logger_.logMeta(SingletonLogger::INFO, "RouteHandler constructed.", __FILE__, __LINE__, __func__);
}

RouteHandler::~RouteHandler() {}

bool RouteHandler::handleNewUser(std::shared_ptr<User> user)
{
    if (!user)
    {
        logger_.logMeta(SingletonLogger::ERROR, "handleNewUser() received null user", __FILE__, __LINE__, __func__);
        return false;
    }

    logger_.logMeta(SingletonLogger::DEBUG, "handleNewUser().", __FILE__, __LINE__, __func__);

    if (userDBManager_->usernameExists(user->getUsername()))
    {
        logger_.logMeta(SingletonLogger::WARNING, "Username already exists: " + user->getUsername(), __FILE__, __LINE__, __func__);
        return false;
    }

    if (!user->getEmail().empty() && userDBManager_->emailExists(user->getEmail()))
    {
        logger_.logMeta(SingletonLogger::WARNING, "Email already exists: " + user->getEmail(), __FILE__, __LINE__, __func__);
        return false;
    }

    bool success = userDBManager_->addUserToDB(user);
    if (success && userKafkaManager_)
    {
        userKafkaManager_->produceNewUser(user);
    }

    return success;
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

nlohmann::json RouteHandler::handleGetUsersPaginated(int offset, int limit)
{
    logger_.logMeta(SingletonLogger::DEBUG, "handleGetUsersPaginated().", __FILE__, __LINE__, __func__);
    return userDBManager_->getUsersPaginated(offset, limit);
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

nlohmann::json RouteHandler::checkUserAvailability(const std::string &username, const std::string &email)
{
    logger_.logMeta(SingletonLogger::DEBUG, "checkUserAvailability().", __FILE__, __LINE__, __func__);
    nlohmann::json result;
    if (!username.empty())
    {
        result["usernameTaken"] = userDBManager_->usernameExists(username);
    }
    if (!email.empty())
    {
        result["emailTaken"] = userDBManager_->emailExists(email);
    }
    return result;
}

nlohmann::json RouteHandler::getUserStats()
{
    logger_.logMeta(SingletonLogger::DEBUG, "getUserStats().", __FILE__, __LINE__, __func__);
    return userDBManager_->getUserStats();
}

nlohmann::json RouteHandler::getHealthStatus()
{
    logger_.logMeta(SingletonLogger::DEBUG, "getHealthStatus().", __FILE__, __LINE__, __func__);

    nlohmann::json status;
    status["service"] = "UserManager";
    status["timestamp_ms"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();

    bool dbHealthy = false;
    double latencyMs = 0.0;

    if (database_)
    {
        auto start = std::chrono::steady_clock::now();
        dbHealthy = database_->executeSelect("SELECT 1;");
        latencyMs = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                        std::chrono::steady_clock::now() - start)
                        .count();
    }

    status["database"] = {
        {"healthy", dbHealthy},
        {"latency_ms", latencyMs}
    };

    nlohmann::json kafkaStatus;
    kafkaStatus["configured"] = static_cast<bool>(kafkaHandler_);
    if (kafkaHandler_)
    {
        kafkaStatus["producer_count"] = kafkaHandler_->getProducers().size();
        kafkaStatus["consumer_count"] = kafkaHandler_->getConsumers().size();
        kafkaStatus["bootstrap"] = UberUtils::CONFIG::getKafkaHost() + ":" + std::to_string(UberUtils::CONFIG::getKafkaPort());
    }
    status["kafka"] = kafkaStatus;

    status["environment"] = {
        {"host", UberUtils::CONFIG::getUserManagerHost()},
        {"database", UberUtils::CONFIG::getUserManagerDatabase()},
        {"http_port", UberUtils::CONFIG::getUserManagerHttpPort()}
    };

    return status;
}
