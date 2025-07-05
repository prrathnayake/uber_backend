#include "../include/sharedKafkaProducer.h"

using namespace utils;
using namespace UberBackend;
using namespace kafka;

SharedKafkaProducer::SharedKafkaProducer(const std::string &producerName,
                                         const std::string &host,
                                         const std::string &port)
    : logger_(SingletonLogger::instance()),
      producerName_(producerName),
      host_(host),
      port_(port)
{
    kafkaProducer_ = new KafkaProducer(host_ + ":" + port_);
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaProducer initialized", __FILE__, __LINE__, __func__);
}

SharedKafkaProducer::~SharedKafkaProducer()
{
    if (kafkaProducer_)
    {
        delete kafkaProducer_;
        kafkaProducer_ = nullptr;
    }
    logger_.logMeta(SingletonLogger::INFO, "SharedKafkaProducer destroyed", __FILE__, __LINE__, __func__);
}

void SharedKafkaProducer::sendMessage(const std::string &topic, const std::string &message)
{
    kafkaProducer_->produceMessages(topic, message);
    logger_.logMeta(SingletonLogger::DEBUG, "Message sent to Kafka topic '" + topic + "': " + message, __FILE__, __LINE__, __func__);
}
