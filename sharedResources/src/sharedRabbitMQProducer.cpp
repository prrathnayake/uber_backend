#include "../include/sharedRabbitMQProducer.h"
#include "../include/sharedRabbitMQHandler.h"

#include <mutex>

using namespace UberBackend;
using namespace utils;

SharedRabbitMQProducer::SharedRabbitMQProducer(const std::string &name,
                                               std::shared_ptr<RabbitQueueState> queue,
                                               SingletonLogger &logger)
    : queue_(queue), logger_(logger), name_(name)
{
    if (name_.empty())
    {
        logger_.logMeta(SingletonLogger::WARNING,
                        "RabbitMQ producer created with an empty name",
                        __FILE__,
                        __LINE__,
                        __func__);
    }
}

void SharedRabbitMQProducer::publish(const std::string &message)
{
    if (message.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "Attempted to publish an empty RabbitMQ message",
                        __FILE__,
                        __LINE__,
                        __func__);
        return;
    }

    if (auto queue = queue_.lock())
    {
        {
            std::lock_guard<std::mutex> lock(queue->mutex);
            queue->messages.push_back(message);
        }
        queue->cv.notify_one();
        logger_.logMeta(SingletonLogger::DEBUG,
                        "RabbitMQ message enqueued on " + queue->name + " by " + name_,
                        __FILE__,
                        __LINE__,
                        __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "Failed to publish RabbitMQ message because the queue no longer exists",
                        __FILE__,
                        __LINE__,
                        __func__);
    }
}

std::string SharedRabbitMQProducer::name() const
{
    return name_;
}

