#include "../include/sharedRabbitMQHandler.h"
#include "../include/sharedRabbitMQConsumer.h"
#include "../include/sharedRabbitMQProducer.h"

#include <sstream>

using namespace UberBackend;
using namespace utils;

SharedRabbitMQHandler::SharedRabbitMQHandler(ConnectionOptions options)
    : logger_(SingletonLogger::instance()),
      threadPool_(&ThreadPool::instance()),
      options_(std::move(options))
{
    std::ostringstream oss;
    oss << "RabbitMQ Handler initialised for host=" << options_.host
        << ":" << options_.port;
    if (!options_.vhost.empty())
    {
        oss << " vhost=" << options_.vhost;
    }
    logger_.logMeta(SingletonLogger::INFO, oss.str(), __FILE__, __LINE__, __func__);
}

SharedRabbitMQHandler::~SharedRabbitMQHandler()
{
    stopConsumers();
    logger_.logMeta(SingletonLogger::INFO, "SharedRabbitMQHandler destroyed", __FILE__, __LINE__, __func__);
}

std::shared_ptr<RabbitQueueState> SharedRabbitMQHandler::getOrCreateQueue(const std::string &queueName)
{
    auto it = queues_.find(queueName);
    if (it != queues_.end())
    {
        if (auto existing = it->second.lock())
        {
            return existing;
        }
    }

    auto queue = std::make_shared<RabbitQueueState>();
    queue->name = queueName;
    queues_[queueName] = queue;

    logger_.logMeta(SingletonLogger::DEBUG,
                    "Created in-memory RabbitMQ queue: " + queueName,
                    __FILE__,
                    __LINE__,
                    __func__);

    return queue;
}

std::shared_ptr<SharedRabbitMQProducer> SharedRabbitMQHandler::createProducer(const std::string &name,
                                                                              const std::string &queueName)
{
    if (name.empty() || queueName.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "Failed to create RabbitMQ producer due to missing name or queue",
                        __FILE__,
                        __LINE__,
                        __func__);
        return nullptr;
    }

    auto queue = getOrCreateQueue(queueName);
    auto producer = std::make_shared<SharedRabbitMQProducer>(name, queue, logger_);
    producers_.push_back(producer);

    logger_.logMeta(SingletonLogger::INFO,
                    "RabbitMQ producer created: " + name + " -> queue " + queueName,
                    __FILE__,
                    __LINE__,
                    __func__);

    return producer;
}

std::shared_ptr<SharedRabbitMQConsumer> SharedRabbitMQHandler::createConsumer(const std::string &name,
                                                                              const std::string &queueName)
{
    if (name.empty() || queueName.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "Failed to create RabbitMQ consumer due to missing name or queue",
                        __FILE__,
                        __LINE__,
                        __func__);
        return nullptr;
    }

    auto queue = getOrCreateQueue(queueName);
    auto consumer = std::make_shared<SharedRabbitMQConsumer>(name, queue, logger_);
    consumers_.push_back(consumer);

    logger_.logMeta(SingletonLogger::INFO,
                    "RabbitMQ consumer created: " + name + " <- queue " + queueName,
                    __FILE__,
                    __LINE__,
                    __func__);

    return consumer;
}

const std::vector<std::shared_ptr<SharedRabbitMQProducer>> &SharedRabbitMQHandler::getProducers() const
{
    return producers_;
}

const std::vector<std::shared_ptr<SharedRabbitMQConsumer>> &SharedRabbitMQHandler::getConsumers() const
{
    return consumers_;
}

void SharedRabbitMQHandler::runConsumers()
{
    for (auto &consumer : consumers_)
    {
        workerFutures_.push_back(
            threadPool_->enqueue([consumer]()
                                  { consumer->listening(); }));
    }
}

void SharedRabbitMQHandler::stopConsumers()
{
    for (auto &consumer : consumers_)
    {
        if (consumer)
        {
            consumer->stop();
        }
    }

    for (auto &queueEntry : queues_)
    {
        if (auto queue = queueEntry.second.lock())
        {
            {
                std::lock_guard<std::mutex> lock(queue->mutex);
                queue->closed = true;
            }
            queue->cv.notify_all();
        }
    }

    for (auto &future : workerFutures_)
    {
        if (future.valid())
        {
            future.get();
        }
    }

    workerFutures_.clear();
}

