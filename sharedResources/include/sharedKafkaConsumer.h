#pragma once

#include <string>
#include <memory>
#include <httplib.h>

#include <utils/index.h>
#include <kafka/index.h>

using namespace utils;

namespace UberBackend
{
    class SharedKafkaConsumer
    {
    public:
        SharedKafkaConsumer(const std::string &consumerName,
                            const std::string &topic,
                            const std::string &host,
                            const std::string &port);
        ~SharedKafkaConsumer();

        void listening();

    protected:
        SingletonLogger &logger_;
        kafka::KafkaConsumer *kafkaConsumer_;

        std::string consumerName_;
        std::string topic_;
        std::string host_;
        std::string port_;
    };
}
