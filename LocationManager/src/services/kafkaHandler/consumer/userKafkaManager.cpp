#include <iostream>
#include <string>

#include <utils/index.h>
#include "../../../../include/services/kafkaHandler/consumer/userKafkaManager.h"

using namespace UberBackend;
using namespace utils;

UserKafkaManager::UserKafkaManager(std::shared_ptr<SharedKafkaConsumer> kafkaConsumer)
    : logger_(SingletonLogger::instance()),
      kafkaConsumer_(kafkaConsumer)
{
}

UserKafkaManager::~UserKafkaManager() {}

std::shared_ptr<User> UserKafkaManager::consumeNewUser()
{
    logger_.logMeta(SingletonLogger::INFO, "Inside: consumeNewUser()", __FILE__, __LINE__, __func__);

    std::string message = kafkaConsumer_->listening();

    logger_.logMeta(SingletonLogger::DEBUG, "Kafka Message : " + message, __FILE__, __LINE__, __func__);
    return nullptr;
}
