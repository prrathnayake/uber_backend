#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <future>


#include <utils/index.h>
#include <kafka/index.h>

#include "sharedKafkaProducer.h"
#include "sharedKafkaConsumer.h"

using namespace utils;

namespace UberBackend
{
    class SharedKafkaHandler
    {
    public:
        SharedKafkaHandler(const std::string &host, const std::string &port);
        virtual ~SharedKafkaHandler();

        std::shared_ptr<SharedKafkaProducer> createProducer(const std::string &name);
        std::shared_ptr<SharedKafkaConsumer> createConsumer(const std::string &name, const std::string &topic);

        const std::vector<std::shared_ptr<SharedKafkaProducer>>& getProducers() const;
        const std::vector<std::shared_ptr<SharedKafkaConsumer>>& getConsumers() const;

        void runConsumers();

    protected:
        SingletonLogger &logger_;
        ThreadPool* thread_pool_;
        std::vector<std::future<void>> kafkaServerFutures_;

        std::vector<std::shared_ptr<SharedKafkaProducer>> kafkaProducers_;
        std::vector<std::shared_ptr<SharedKafkaConsumer>> kafkaConsumers_;

        const std::string host_;
        const std::string port_;
    };
}
