#include <iostream>
#include <string>

#include <utils/index.h>
#include "../../../../include/services/kafkaHandler/producer/userKafkaManager.h"

using namespace UberBackend;
using namespace utils;

UserKafkaManager::UserKafkaManager()
    : logger_(SingletonLogger::instance())
{
    kafkaProducer_ = std::make_shared<SharedKafkaProducer>(
        "UserProducer",
        "localhost",
        "9092");
    topic_ = "user";
}

UserKafkaManager::~UserKafkaManager() {}

void UserKafkaManager::produceNewUser(std::shared_ptr<User> user)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside: produceNewUser()", __FILE__, __LINE__, __func__);

    // Create a structured JSON message from the User object
    nlohmann::json messageJson;
    messageJson["event"] = "user_created";
    messageJson["data"] = {
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
        {"country", user->getCountry()}};

    std::string message = messageJson.dump(); // Convert to string

    logger_.logMeta(SingletonLogger::DEBUG, "Kafka Message: " + message, __FILE__, __LINE__, __func__);

    kafkaProducer_->sendMessage(topic_, message);
}
