#include <iostream>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

Server::Server(const std::string &serverName,
               const std::string &host,
               const std::string &user,
               const std::string &password,
               const std::string &dbName,
               unsigned int port)
    : SharedServer(serverName, host, user, password, dbName, port)
{
}

void Server::createHttpServers()
{
    logger_.logMeta(SingletonLogger::INFO, "create HTTP Server Handler.", __FILE__, __LINE__, __func__);

    // implementing the HttpHandler to handle HTTP requests and passing the database details
    httpServerHandler_ = std::make_unique<HttpHandler>(database_);
    logger_.logMeta(SingletonLogger::DEBUG, "run : httpServerHandler_->createServers();", __FILE__, __LINE__, __func__);
    httpServerHandler_->createServers();
}

void Server::startConsumers()
{
    if (!sharedKafkaHandler_)
    {
        auto kafkaHost = UberUtils::CONFIG::getKafkaHost();
        auto kafkaPort = UberUtils::CONFIG::getKafkaPort();
        sharedKafkaHandler_ = std::make_unique<SharedKafkaHandler>(kafkaHost, std::to_string(kafkaPort));
    }

    auto kafkaConsumer = sharedKafkaHandler_->createConsumer("user_manager_profile_events", "user_profile_updated");
    if (kafkaConsumer)
    {
        kafkaConsumer->setCallback([](const std::string &payload)
                                   { std::cout << "[Kafka][UserManager] profile update event -> " << payload << std::endl; });
    }

    sharedKafkaHandler_->runConsumers();

    SharedRabbitMQHandler::ConnectionOptions rabbitOptions{
        UberUtils::CONFIG::getRabbitMQHost(),
        std::to_string(UberUtils::CONFIG::getRabbitMQPort()),
        UberUtils::CONFIG::getRabbitMQUsername(),
        UberUtils::CONFIG::getRabbitMQPassword(),
        UberUtils::CONFIG::getRabbitMQVHost()};

    if (!sharedRabbitHandler_)
    {
        sharedRabbitHandler_ = std::make_unique<SharedRabbitMQHandler>(rabbitOptions);
    }

    auto taskConsumer = sharedRabbitHandler_->createConsumer("user_manager_tasks", "user_tasks");
    if (taskConsumer)
    {
        taskConsumer->setCallback([](const std::string &task)
                                  { std::cout << "[RabbitMQ][UserManager] task received -> " << task << std::endl; });
    }

    sharedRabbitHandler_->runConsumers();
}