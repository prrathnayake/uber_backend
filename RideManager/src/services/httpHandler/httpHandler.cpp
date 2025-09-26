#include "../../../include/services/httpHandler/httpHandler.h"

#include <memory>

#include "../../../include/services/httpHandler/servers/httpRideServer.h"
#include "../../../../sharedUtils/include/config.h"

namespace UberBackend
{
    HttpHandler::HttpHandler(std::shared_ptr<SharedDatabase> db)
        : SharedHttpHandler(std::move(db))
    {
    }

    void HttpHandler::createServers()
    {
        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Creating RideManager HTTP server",
                        __FILE__,
                        __LINE__,
                        __func__);

        auto rideServer = std::make_unique<HttpRideServer>(
            "ride_http_handler",
            UberUtils::CONFIG::getRideManagerHost(),
            static_cast<int>(UberUtils::CONFIG::getRideManagerHttpPort()),
            database_);
        rideServer->createServerMethods();
        servers_.push_back(std::move(rideServer));
    }
}
