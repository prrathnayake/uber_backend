#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include <utils/index.h>

namespace UberBackend
{
    class SharedRabbitMQHandler;
    struct RabbitQueueState;

    class SharedRabbitMQConsumer
    {
    public:
        using Callback = std::function<void(const std::string &)>;

        SharedRabbitMQConsumer(const std::string &name,
                               std::shared_ptr<RabbitQueueState> queue,
                               utils::SingletonLogger &logger);
        ~SharedRabbitMQConsumer();

        void setCallback(Callback callback);
        void listening();
        void stop();

        [[nodiscard]] std::string name() const;
        [[nodiscard]] std::string queueName() const;

    private:
        std::weak_ptr<RabbitQueueState> queue_;
        utils::SingletonLogger &logger_;
        Callback callback_;
        std::atomic<bool> running_{true};
        std::string name_;
        std::string queueName_;
    };
}

