#include "../include/server.h"
#include "../../sharedUtils/include/config.h"
#include "../../sharedResources/include/sharedRabbitMQConsumer.h"


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

    httpServerHandler_ = std::make_unique<HttpHandler>(database_);
    httpServerHandler_->createServers();

    const auto grpcPort = UberUtils::CONFIG::getLocationManagerGrpcPort();
    const std::string grpcAddress = "0.0.0.0:" + std::to_string(grpcPort);
    ensureGrpcServer(grpcAddress);
    startGrpcServer();
}

void Server::startConsumers()
{
    std::string host = UberUtils::CONFIG::getKafkaHost();
    unsigned int port = UberUtils::CONFIG::getKafkaPort();

    if (!sharedKafkaHandler_)
    {
        sharedKafkaHandler_ = std::make_unique<SharedKafkaHandler>(host, std::to_string(port));
    }

    std::string name = "newUser";
    std::string topic = "user_created";

    std::shared_ptr<SharedKafkaConsumer> kafkaConsumer = sharedKafkaHandler_->createConsumer(name, topic);
    kafkaConsumer->setCallback([this](const std::string &msg)
                               {
                                   logger_.logMeta(SingletonLogger::INFO,
                                                   std::string{"[Kafka][LocationManager] message -> "} + msg,
                                                   __FILE__,
                                                   __LINE__,
                                                   __func__);
                               });

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

    auto rabbitConsumer = sharedRabbitHandler_->createConsumer("location_events", "location_updates");
    if (rabbitConsumer)
    {
        rabbitConsumer->setCallback([this](const std::string &message)
                                    {
                                        logger_.logMeta(SingletonLogger::INFO,
                                                        std::string{"[RabbitMQ][LocationManager] payload -> "} + message,
                                                        __FILE__,
                                                        __LINE__,
                                                        __func__);
                                    });
    }

    sharedRabbitHandler_->runConsumers();
}
