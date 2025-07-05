#pragma once

#include <string>
#include <memory>
#include <httplib.h>

#include <utils/index.h>
#include <kafka/index.h>

using namespace utils;

namespace UberBackend
{
    class SharedKafkaProducer
    {
    public:
        SharedKafkaProducer(const std::string &producerName,
                            const std::string &host,
                            const std::string &port);
        ~SharedKafkaProducer();

        void sendMessage(const std::string &topic, const std::string &message);

    protected:
        SingletonLogger &logger_;
        kafka::KafkaProducer *kafkaProducer_;

        std::string producerName_;
        std::string host_;
        std::string port_;
    };
}
