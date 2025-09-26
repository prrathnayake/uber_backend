#include "../include/server.h"

#include <iostream>

#include "../../sharedResources/include/sharedKafkaHandler.h"
#include "../../sharedResources/include/sharedRabbitMQConsumer.h"
#include "../../sharedResources/include/sharedRabbitMQHandler.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;

namespace UberBackend
{
    RideServer::RideServer(const std::string &serverName,
                           const std::string &host,
                           const std::string &user,
                           const std::string &password,
                           const std::string &dbName,
                           unsigned int port)
        : SharedServer(serverName, host, user, password, dbName, port)
    {
    }

    void RideServer::createHttpServers()
    {
        logger_.logMeta(SingletonLogger::INFO, "Initialising RideManager HTTP handlers", __FILE__, __LINE__, __func__);

        httpServerHandler_ = std::make_unique<HttpHandler>(database_);
        httpServerHandler_->createServers();
    }

    void RideServer::startConsumers()
    {
        startKafkaConsumers();
        startRabbitConsumers();
    }

    void RideServer::startKafkaConsumers()
    {
        const auto kafkaHost = UberUtils::CONFIG::getKafkaHost();
        const auto kafkaPort = UberUtils::CONFIG::getKafkaPort();

        if (!sharedKafkaHandler_)
        {
            logger_.logMeta(SingletonLogger::INFO,
                            "Bootstrapping Kafka handler for RideManager",
                            __FILE__,
                            __LINE__,
                            __func__);
            sharedKafkaHandler_ = std::make_unique<SharedKafkaHandler>(kafkaHost, std::to_string(kafkaPort));
        }

        auto rideLifecycleConsumer = sharedKafkaHandler_->createConsumer("ride_manager_lifecycle", "ride_lifecycle_events");
        if (rideLifecycleConsumer)
        {
            rideLifecycleConsumer->setCallback([](const std::string &payload)
                                               {
                                                   std::cout << "[Kafka][RideManager] lifecycle event -> " << payload << std::endl;
                                               });
        }

        auto locationBridgeConsumer = sharedKafkaHandler_->createConsumer("ride_manager_location_bridge", "location_events");
        if (locationBridgeConsumer)
        {
            locationBridgeConsumer->setCallback([](const std::string &payload)
                                                {
                                                    std::cout << "[Kafka][RideManager] location update -> " << payload << std::endl;
                                                });
        }

        sharedKafkaHandler_->runConsumers();
    }

    void RideServer::startRabbitConsumers()
    {
        SharedRabbitMQHandler::ConnectionOptions options{
            UberUtils::CONFIG::getRabbitMQHost(),
            std::to_string(UberUtils::CONFIG::getRabbitMQPort()),
            UberUtils::CONFIG::getRabbitMQUsername(),
            UberUtils::CONFIG::getRabbitMQPassword(),
            UberUtils::CONFIG::getRabbitMQVHost()};

        if (!sharedRabbitHandler_)
        {
            logger_.logMeta(SingletonLogger::INFO,
                            "Initialising RabbitMQ handler for RideManager",
                            __FILE__,
                            __LINE__,
                            __func__);
            sharedRabbitHandler_ = std::make_unique<SharedRabbitMQHandler>(options);
        }

        auto driverTaskConsumer = sharedRabbitHandler_->createConsumer("ride_manager_driver_tasks", "driver_notifications");
        if (driverTaskConsumer)
        {
            driverTaskConsumer->setCallback([](const std::string &message)
                                            {
                                                std::cout << "[RabbitMQ][RideManager] driver task -> " << message << std::endl;
                                            });
        }

        sharedRabbitHandler_->runConsumers();
    }
}
