#include <iostream>
#include <string>

#include <utils/index.h>
#include "../../../../include/services/kafkaHandler/producer/userKafkaManager.h"

using namespace UberBackend;
using namespace utils;

UserKafkaManager::UserKafkaManager(std::shared_ptr<SharedKafkaProducer> kafkaProducer)
    : logger_(SingletonLogger::instance()),
      kafkaProducer_(kafkaProducer)
{
}

UserKafkaManager::~UserKafkaManager() {}

void UserKafkaManager::sendEvent(const std::string &eventType, const nlohmann::json &data)
{
    nlohmann::json messageJson;
    messageJson["event"] = eventType;
    messageJson["data"] = data;

    std::string message = messageJson.dump();
    logger_.logMeta(SingletonLogger::DEBUG, "Kafka Message [" + eventType + "]: " + message, __FILE__, __LINE__, __func__);

    kafkaProducer_->sendMessage(topic_, message);
}

void UserKafkaManager::produceNewUser(std::shared_ptr<User> user)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside: produceNewUser()", __FILE__, __LINE__, __func__);

    nlohmann::json data = {
        {"first_name", user->getFirstName()},
        {"middle_name", user->getMiddleName()},
        {"last_name", user->getLastName()},
        {"username", user->getUsername()},
        {"email", user->getEmail()},
        {"mobile_number", user->getMobileNumber()},
        {"address", user->getAddress()},
        {"role", user->getRole()},
        {"country_code", user->getCountryCode()},
        {"preferred_language", user->getPreferredLanguage()},
        {"currency", user->getCurrency()},
        {"country", user->getCountry()}
    };

    sendEvent("user_created", data);
}

void UserKafkaManager::produceUpdatedUser(std::shared_ptr<User> user)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside: produceUpdatedUser()", __FILE__, __LINE__, __func__);

    nlohmann::json data = {
        {"username", user->getUsername()},
        {"email", user->getEmail()},
        {"role", user->getRole()}
        // Add more fields if needed
    };

    sendEvent("user_updated", data);
}

void UserKafkaManager::produceDeletedUser(const std::string &userId)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside: produceDeletedUser()", __FILE__, __LINE__, __func__);

    nlohmann::json data = {
        {"user_id", userId}
    };

    sendEvent("user_deleted", data);
}
