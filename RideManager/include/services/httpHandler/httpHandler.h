#pragma once

#include <memory>

#include "../../../../sharedResources/include/sharedHTTPHandler.h"

namespace UberBackend
{
    class HttpHandler : public SharedHttpHandler
    {
    public:
        explicit HttpHandler(std::shared_ptr<SharedDatabase> db);
        void createServers() override;
    };
}
