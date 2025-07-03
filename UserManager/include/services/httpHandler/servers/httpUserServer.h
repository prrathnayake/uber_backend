#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include "../../../../../sharedResources/include/sharedHTTPServer.h"
#include "../../../database/userDBManager.h"

namespace UberBackend
{
    class HttpUserServer : public SharedHttpServer
    {
    public:
    HttpUserServer(const std::string& name,
               const std::string& host,
               int port,
               std::shared_ptr<SharedDatabase> db);

        void createServerMethods() override;

    private:
        std::shared_ptr<UberBackend::UserDBManager> userDBManager_;
    };
}
