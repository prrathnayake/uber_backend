#pragma once

#include <memory>
#include <future>

#include <utils/index.h>
#include <database/index.h>

// #include "./services/httpHandler/httpHandler.h"
#include "../../sharedResources/include/sharedServer.h"

namespace UberBackend
{
    class Server : public SharedServer
    {
    public:
        Server(const std::string &serverName,
               const std::string &host,
               const std::string &user,
               const std::string &password,
               const std::string &dbName,
               unsigned int port);

        void createHttpServers() override;

    private:
    };

};
