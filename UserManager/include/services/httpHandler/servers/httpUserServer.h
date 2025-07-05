#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>
#include <utils/index.h>
#include <algorithms/sha256/index.h>

#include "../../../../../sharedResources/include/sharedHTTPServer.h"
#include "../../../../../sharedUtils/include/jwt.h"
#include "../../../database/userDBManager.h"

namespace UberBackend
{
    class HttpUserServer : public SharedHttpServer
    {
    public:
        HttpUserServer(const std::string &name,
                       const std::string &host,
                       int port,
                       std::shared_ptr<SharedDatabase> db);

        void createServerMethods() override;

    private:
        JWTUtils jwt_;
        std::shared_ptr<UberBackend::UserDBManager> userDBManager_;
    };
}