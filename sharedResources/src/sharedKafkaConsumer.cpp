#include "../include/sharedKafkaConsumer.h"

#include <chrono>

using namespace utils;
using namespace UberBackend;
using namespace kafka;

SharedKafkaConsumer::SharedKafkaConsumer(const std::string &consumerName,
                                         const std::string &topic,
                                         const std::string &host,
                                         const std::string &port)
    : logger_(SingletonLogger::instance()),
      kafkaConsumer_(nullptr),
      consumerName_(consumerName),
      topic_(topic),
      host_(host),
      port_(port)
{
    if (consumerName_.empty() || topic_.empty() || host_.empty() || port_.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "createConsumer failed due to not having proper consumerName, topic, host or port.",
                        __FILE__,
                        __LINE__,
                        __func__);
        return;
    }

    logger_.logMeta(SingletonLogger::INFO, host_ + ":" + port_, __FILE__, __LINE__, __func__);
    kafkaConsumer_ = new KafkaConsumer(host_ + ":" + port_, topic_);
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaConsumer initialized", __FILE__, __LINE__, __func__);
}

SharedKafkaConsumer::~SharedKafkaConsumer()
{
    stop();

    if (kafkaConsumer_)
    {
        delete kafkaConsumer_;
        kafkaConsumer_ = nullptr;
    }
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaConsumer destroyed", __FILE__, __LINE__, __func__);
}

void SharedKafkaConsumer::setCallback(std::function<void(const std::string &)> callback)
{
    callback_ = std::move(callback);
}

std::string SharedKafkaConsumer::listening()
{
    if (!kafkaConsumer_)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Kafka consumer is not initialised", __FILE__, __LINE__, __func__);
        return {};
    }

    std::string lastMessage;
    while (shouldRun_.load())
    {
        logger_.logMeta(SingletonLogger::DEBUG,
                        std::string("Kafka consumer active: ") + (shouldRun_.load() ? "true" : "false"),
                        __FILE__,
                        __LINE__,
                        __func__);

        std::string message = kafkaConsumer_->consumeMessage();
        if (!message.empty())
        {
            lastMessage = message;
            if (callback_)
            {
                callback_(message);
            }
        }

        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(100), [this] { return !shouldRun_.load(); });
    }

    logger_.logMeta(SingletonLogger::INFO, "Kafka consumer stopped: " + consumerName_, __FILE__, __LINE__, __func__);
    return lastMessage;
}

void SharedKafkaConsumer::stop()
{
    bool expected = true;
    if (shouldRun_.compare_exchange_strong(expected, false))
    {
        if (kafkaConsumer_)
        {
            kafkaConsumer_->stopConsumeMessages();
        }
        cv_.notify_all();
    }
}
