#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <utils/index.h>

namespace kafka {

class KafkaBroker;

class KafkaProducer {
public:
    explicit KafkaProducer(std::string bootstrapServers);
    void produceMessages(const std::string &topic, const std::string &message);

private:
    std::string bootstrapServers_;
    utils::SingletonLogger &logger_;
};

class KafkaConsumer {
public:
    KafkaConsumer(std::string bootstrapServers, std::string topic);
    ~KafkaConsumer();

    std::string consumeMessage();
    void stopConsumeMessages();

private:
    std::string bootstrapServers_;
    std::string topic_;
    bool stopRequested_{false};
    utils::SingletonLogger &logger_;
};

} // namespace kafka

