#pragma once

#include <string>
#include <memory>
#include <atomic>
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

        std::string listening();
        void stop();
        void setCallback(std::function<void(const std::string &)> callback);

    protected:
        SingletonLogger &logger_;
        kafka::KafkaConsumer *kafkaConsumer_;
        std::atomic<bool> shouldRun_;
        std::function<void(const std::string &)> callback_;

        std::string consumerName_;
        std::string topic_;
        std::string host_;
        std::string port_;
    };
}
