#include "../include/sharedKafkaConsumer.h"

using namespace utils;
using namespace UberBackend;
using namespace kafka;

SharedKafkaConsumer::SharedKafkaConsumer(const std::string &consumerName,
                                         const std::string &topic,
                                         const std::string &host,
                                         const std::string &port)
    : logger_(SingletonLogger::instance()),
      consumerName_(consumerName),
      topic_(topic),
      host_(host),
      port_(port)
{
    kafkaConsumer_ = new KafkaConsumer(host_ + ":" + port_, topic_);
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaConsumer initialized", __FILE__, __LINE__, __func__);
}

SharedKafkaConsumer::~SharedKafkaConsumer()
{
    if (kafkaConsumer_)
    {
        delete kafkaConsumer_;
        kafkaConsumer_ = nullptr;
    }
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaConsumer destroyed", __FILE__, __LINE__, __func__);
}

void SharedKafkaConsumer::listening()
{
    logger_.logMeta(SingletonLogger::INFO, "Kafka Consumer started listening...", __FILE__, __LINE__, __func__);
    ExCosumeCb cb;
    kafkaConsumer_->consumeMessages(cb);
}
