#pragma once

#include <memory>
#include <vector>

#include <utils/index.h>
#include <database/database.h>

#include "../../../../sharedResources/include/httplib.h"
#include "../../../../sharedResources/include/sharedHTTPHandler.h"
#include "./servers/httpServer.h"

using namespace UberBackend;

namespace UberBackend
{

    class HttpHandler : public SharedHttpHandler
    {
    public:
        HttpHandler(std::shared_ptr<SharedDatabase> db);
        void createServers() override;
    };

}
