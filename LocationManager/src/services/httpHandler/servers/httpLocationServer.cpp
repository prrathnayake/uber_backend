#include "services/httpHandler/servers/httpLocationServer.h"

#include <nlohmann/json.hpp>

#include "config.h"

using namespace UberBackend;
using namespace utils;
using nlohmann::json;

HttpLocationServer::HttpLocationServer(const std::string &name,
                                       const std::string &host,
                                       int port,
                                       std::shared_ptr<SharedDatabase> db)
    : SharedHttpServer(name, host, port, db),
      jwt_(UberUtils::CONFIG::getJwtSecret()),
      routeHandler_(std::make_shared<RouteHandler>(db))
{
}

void HttpLocationServer::createServerMethods()
{
    server_->Post("/location/update", [this](const httplib::Request &req, httplib::Response &res) {
        json payload;
        try
        {
            payload = json::parse(req.body);
        }
        catch (const json::parse_error &)
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid JSON payload"})", "application/json");
            return;
        }

        auto result = routeHandler_->handleLocationUpdate(payload);
        int statusCode = result.value("http_status", 500);
        result.erase("http_status");
        res.status = statusCode;
        res.set_content(result.dump(), "application/json");
    });

    server_->Get(R"(/location/(\w+))", [this](const httplib::Request &req, httplib::Response &res) {
        std::string userId = req.matches[1];
        auto result = routeHandler_->handleGetLocationByUserId(userId);
        int statusCode = result.value("http_status", 500);
        result.erase("http_status");
        res.status = statusCode;
        res.set_content(result.dump(), "application/json");
    });

    server_->Get("/location/nearby", [this](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("lat") || !req.has_param("lng"))
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing query params"})", "application/json");
            return;
        }

        double lat = 0.0;
        double lng = 0.0;
        double radius = 5.0;
        std::string role = req.has_param("role") ? req.get_param_value("role") : "driver";

        try
        {
            lat = std::stod(req.get_param_value("lat"));
            lng = std::stod(req.get_param_value("lng"));
            if (req.has_param("radius"))
            {
                radius = std::stod(req.get_param_value("radius"));
            }
        }
        catch (const std::exception &)
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Query parameters must be numeric"})", "application/json");
            return;
        }

        auto result = routeHandler_->handleFindNearby(lat, lng, role, radius);
        int statusCode = result.value("http_status", 500);
        result.erase("http_status");
        res.status = statusCode;
        res.set_content(result.dump(), "application/json");
    });

    server_->Delete(R"(/location/(\w+))", [this](const httplib::Request &req, httplib::Response &res) {
        std::string userId = req.matches[1];
        auto result = routeHandler_->handleDeleteLocation(userId);
        int statusCode = result.value("http_status", 500);
        result.erase("http_status");
        res.status = statusCode;
        res.set_content(result.dump(), "application/json");
    });
}
