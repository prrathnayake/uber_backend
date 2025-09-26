#include "../include/sharedRabbitMQConsumer.h"
#include "../include/sharedRabbitMQHandler.h"

#include <chrono>
#include <mutex>

using namespace UberBackend;
using namespace utils;

SharedRabbitMQConsumer::SharedRabbitMQConsumer(const std::string &name,
                                               std::shared_ptr<RabbitQueueState> queue,
                                               SingletonLogger &logger)
    : queue_(queue),
      logger_(logger),
      name_(name)
{
    if (queue)
    {
        queueName_ = queue->name;
    }

    if (name_.empty())
    {
        logger_.logMeta(SingletonLogger::WARNING,
                        "RabbitMQ consumer created with an empty name",
                        __FILE__,
                        __LINE__,
                        __func__);
    }
}

SharedRabbitMQConsumer::~SharedRabbitMQConsumer()
{
    stop();
}

void SharedRabbitMQConsumer::setCallback(Callback callback)
{
    callback_ = std::move(callback);
}

void SharedRabbitMQConsumer::listening()
{
    while (running_.load())
    {
        std::string message;
        if (auto queue = queue_.lock())
        {
            std::unique_lock<std::mutex> lock(queue->mutex);
            queue->cv.wait(lock, [this, &queue]
                            {
                                return !running_.load() || !queue->messages.empty() || queue->closed;
                            });

            if (!running_.load())
            {
                break;
            }

            if (queue->closed)
            {
                logger_.logMeta(SingletonLogger::INFO,
                                "RabbitMQ queue closed: " + queue->name,
                                __FILE__,
                                __LINE__,
                                __func__);
                break;
            }

            if (!queue->messages.empty())
            {
                message = std::move(queue->messages.front());
                queue->messages.pop_front();
            }
        }

        if (!message.empty())
        {
            if (callback_)
            {
                callback_(message);
            }
            else
            {
                logger_.logMeta(SingletonLogger::WARNING,
                                "RabbitMQ consumer received message without callback",
                                __FILE__,
                                __LINE__,
                                __func__);
            }
        }
    }
}

void SharedRabbitMQConsumer::stop()
{
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false))
    {
        if (auto queue = queue_.lock())
        {
            queue->cv.notify_all();
        }
    }
}

std::string SharedRabbitMQConsumer::name() const
{
    return name_;
}

std::string SharedRabbitMQConsumer::queueName() const
{
    return queueName_;
}

