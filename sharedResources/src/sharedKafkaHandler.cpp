#include "../include/sharedKafkaHandler.h"

using namespace UberBackend;
using namespace utils;
using namespace kafka;

SharedKafkaHandler::SharedKafkaHandler(const std::string &host, const std::string &port)
    : logger_(SingletonLogger::instance()), host_(host), port_(port)
{
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaHandler initialized", __FILE__, __LINE__, __func__);
}

SharedKafkaHandler::~SharedKafkaHandler()
{
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaHandler destroyed", __FILE__, __LINE__, __func__);
}

std::shared_ptr<SharedKafkaProducer> SharedKafkaHandler::createProducer(const std::string &name, const std::string &topic)
{
    auto producer = std::make_shared<SharedKafkaProducer>(name, host_, port_);
    kafkaProducers_.emplace_back(producer);
    logger_.logMeta(SingletonLogger::INFO, "Kafka producer created: " + name, __FILE__, __LINE__, __func__);
    return producer;
}

std::shared_ptr<SharedKafkaConsumer> SharedKafkaHandler::createConsumer(const std::string &name, const std::string &topic)
{
    auto consumer = std::make_shared<SharedKafkaConsumer>(name, topic, host_, port_);
    kafkaConsumers_.emplace_back(consumer);
    logger_.logMeta(SingletonLogger::INFO, "Kafka consumer created: " + name, __FILE__, __LINE__, __func__);
    return consumer;
}

const std::vector<std::shared_ptr<SharedKafkaProducer>>& SharedKafkaHandler::getProducers() const
{
    return kafkaProducers_;
}

const std::vector<std::shared_ptr<SharedKafkaConsumer>>& SharedKafkaHandler::getConsumers() const
{
    return kafkaConsumers_;
}
