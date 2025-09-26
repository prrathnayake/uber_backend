#pragma once

#include <memory>

#include <utils/index.h>

#include "./services/httpHandler/httpHandler.h"
#include "../../sharedResources/include/sharedServer.h"

namespace UberBackend
{
    class RideServer : public SharedServer
    {
    public:
        RideServer(const std::string &serverName,
                   const std::string &host,
                   const std::string &user,
                   const std::string &password,
                   const std::string &dbName,
                   unsigned int port);

        void createHttpServers() override;
        void startConsumers() override;

    private:
        void startKafkaConsumers();
        void startRabbitConsumers();
    };
}
