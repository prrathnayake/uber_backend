#pragma once

#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include <httplib.h>

#include "../../../../../sharedResources/include/sharedHTTPServer.h"
#include "../../routeHandler/rideRouteHandler.h"

namespace UberBackend
{
    class HttpRideServer : public SharedHttpServer
    {
    public:
        HttpRideServer(const std::string &name,
                       const std::string &host,
                       int port,
                       std::shared_ptr<SharedDatabase> db);

        void createServerMethods() override;

    private:
        nlohmann::json parseJsonBody(const httplib::Request &req, httplib::Response &res) const;
        void respondJson(httplib::Response &res, const nlohmann::json &payload, int status = 200) const;

        std::shared_ptr<RideRouteHandler> routeHandler_;
    };
}
