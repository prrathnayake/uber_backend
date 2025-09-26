#pragma once

#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <utils/index.h>

namespace UberBackend
{
    class SharedRabbitMQProducer;
    class SharedRabbitMQConsumer;

    /**
     * @brief Thread-safe in-memory emulation of RabbitMQ primitives.
     *
     * The production deployment connects to a dedicated RabbitMQ cluster via
     * the `app_rabbitmq` library.  The goal of this handler is to provide a
     * deterministic, dependency-light implementation that mirrors the API
     * expected by the microservices while running in local environments or
     * during unit tests where an actual broker is not available.
     */
    struct RabbitQueueState
    {
        std::string name;
        std::deque<std::string> messages;
        std::mutex mutex;
        std::condition_variable cv;
        bool closed{false};
    };

    class SharedRabbitMQHandler
    {
    public:
        struct ConnectionOptions
        {
            std::string host;
            std::string port;
            std::string username;
            std::string password;
            std::string vhost;
        };

        explicit SharedRabbitMQHandler(ConnectionOptions options);
        ~SharedRabbitMQHandler();

        std::shared_ptr<SharedRabbitMQProducer> createProducer(const std::string &name,
                                                               const std::string &queueName);

        std::shared_ptr<SharedRabbitMQConsumer> createConsumer(const std::string &name,
                                                               const std::string &queueName);

        const std::vector<std::shared_ptr<SharedRabbitMQProducer>> &getProducers() const;
        const std::vector<std::shared_ptr<SharedRabbitMQConsumer>> &getConsumers() const;

        void runConsumers();
        void stopConsumers();

    private:
        std::shared_ptr<RabbitQueueState> getOrCreateQueue(const std::string &queueName);

        utils::SingletonLogger &logger_;
        utils::ThreadPool *threadPool_;
        std::vector<std::future<void>> workerFutures_;

        ConnectionOptions options_;
        std::unordered_map<std::string, std::weak_ptr<RabbitQueueState>> queues_;
        std::vector<std::shared_ptr<SharedRabbitMQProducer>> producers_;
        std::vector<std::shared_ptr<SharedRabbitMQConsumer>> consumers_;
    };
}

