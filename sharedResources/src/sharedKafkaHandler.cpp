#include "../include/sharedKafkaHandler.h"

using namespace UberBackend;
using namespace utils;
using namespace kafka;

SharedKafkaHandler::SharedKafkaHandler(const std::string &host, const std::string &port)
    : logger_(SingletonLogger::instance()), thread_pool_(&ThreadPool::instance()), host_(host), port_(port)
{
    if (host_.empty() || port_.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "SharedKafkaProducer initialized failed due to not having proper host or port.", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::INFO, "SharedKafkaHandler initialized", __FILE__, __LINE__, __func__);
    }
}

SharedKafkaHandler::~SharedKafkaHandler()
{
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaHandler destroyed", __FILE__, __LINE__, __func__);
}

std::shared_ptr<SharedKafkaProducer> SharedKafkaHandler::createProducer(const std::string &name)
{
    if (name.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "createProducer failed due to not having proper name.", __FILE__, __LINE__, __func__);
        return nullptr;
    }
    else
    {
        auto producer = std::make_shared<SharedKafkaProducer>(name, host_, port_);
        kafkaProducers_.emplace_back(producer);
        logger_.logMeta(SingletonLogger::INFO, "Kafka producer created: " + name, __FILE__, __LINE__, __func__);
        return producer;
    }
    return nullptr;
}

std::shared_ptr<SharedKafkaConsumer> SharedKafkaHandler::createConsumer(const std::string &name, const std::string &topic)
{
    if (name.empty() || topic.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "createConsumer failed due to not having proper name or topic.", __FILE__, __LINE__, __func__);
        return nullptr;
    }
    else
    {
        auto consumer = std::make_shared<SharedKafkaConsumer>(name, topic, host_, port_);
        kafkaConsumers_.emplace_back(consumer);
        logger_.logMeta(SingletonLogger::INFO, "Kafka consumer created: " + name, __FILE__, __LINE__, __func__);
        return consumer;
    }
    return nullptr;
}

const std::vector<std::shared_ptr<SharedKafkaProducer>> &SharedKafkaHandler::getProducers() const
{
    return kafkaProducers_;
}

const std::vector<std::shared_ptr<SharedKafkaConsumer>> &SharedKafkaHandler::getConsumers() const
{
    return kafkaConsumers_;
}

void SharedKafkaHandler::runConsumers()
{
    for (auto &consumer : kafkaConsumers_)
    {
        kafkaServerFutures_.push_back(
            thread_pool_->enqueue([consumer]()
                                  { consumer->listening(); }));
    }
}

void SharedKafkaHandler::stopConsumers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping all Kafka consumers...", __FILE__, __LINE__, __func__);

    for (auto &consumer : kafkaConsumers_)
    {
        if (consumer)
        {
            consumer->stop(); // Ensure this method is defined in SharedKafkaConsumer
            logger_.logMeta(SingletonLogger::DEBUG, "Stop signal sent to consumer", __FILE__, __LINE__, __func__);
        }
    }

    // Optionally wait for futures to complete if your threads run async
    for (auto &future : kafkaServerFutures_)
    {
        if (future.valid())
        {
            future.get(); // Block until consumer thread finishes
        }
    }

    logger_.logMeta(SingletonLogger::INFO, "All Kafka consumers stopped", __FILE__, __LINE__, __func__);
}
