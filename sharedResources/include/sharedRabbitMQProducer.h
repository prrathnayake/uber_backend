#pragma once

#include <memory>
#include <string>

#include <utils/index.h>

namespace UberBackend
{
    class SharedRabbitMQHandler;
    struct RabbitQueueState;

    class SharedRabbitMQProducer
    {
    public:
        SharedRabbitMQProducer(const std::string &name,
                               std::shared_ptr<RabbitQueueState> queue,
                               utils::SingletonLogger &logger);

        void publish(const std::string &message);
        [[nodiscard]] std::string name() const;

    private:
        std::weak_ptr<RabbitQueueState> queue_;
        utils::SingletonLogger &logger_;
        std::string name_;
    };
}

