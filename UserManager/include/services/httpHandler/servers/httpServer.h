#pragma once

#include <string>
#include <memory>
#include <thread>
#include <httplib.h>

#include <utils/index.h>

#include "../../../../../sharedResources/include/sharedHTTPServer.h"

namespace UberBackend
{
    class HttpServer : public SharedHttpServer
    {
    public:
        void createServerMethods() override;
    };
}
