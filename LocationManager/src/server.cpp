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

    httpServerHandler_ = std::make_unique<HttpHandler>(database_);
    httpServerHandler_->createServers();
}

void Server::startConsumers()
{
    std::string host = UberUtils::CONFIG::KAFKA_HOST;
    int port = UberUtils::CONFIG::KAFKA_PORT;

    if (!sharedKafkaHandler_)
    {
        sharedKafkaHandler_ = std::make_unique<SharedKafkaHandler>(host, std::to_string(port));
    }

    std::string name = "newUser";
    std::string topic = "user_created";

    std::shared_ptr<SharedKafkaConsumer> kafkaConsumer = sharedKafkaHandler_->createConsumer(name, topic);
    kafkaConsumer->setCallback([](const std::string &msg)
                               { std::cout << "[Kafka Msg] " << msg << std::endl; });

    sharedKafkaHandler_->runConsumers();
}
