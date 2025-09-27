#include "../../../../include/services/httpHandler/servers/httpRideServer.h"

#include <algorithm>
#include <optional>

#include <nlohmann/json.hpp>

using nlohmann::json;

namespace UberBackend
{
    HttpRideServer::HttpRideServer(const std::string &name,
                                   const std::string &host,
                                   int port,
                                   std::shared_ptr<SharedDatabase> db)
        : SharedHttpServer(name, host, port, std::move(db)),
          routeHandler_(std::make_shared<RideRouteHandler>(database_))
    {
    }

    json HttpRideServer::parseJsonBody(const httplib::Request &req, httplib::Response &res) const
    {
        if (req.body.empty())
        {
            respondJson(res, json{{"status", "error"}, {"message", "Empty request body"}}, 400);
            return {};
        }

        json payload = json::parse(req.body, nullptr, false);
        if (payload.is_discarded())
        {
            respondJson(res, json{{"status", "error"}, {"message", "Invalid JSON payload"}}, 400);
            return {};
        }

        return payload;
    }

    void HttpRideServer::respondJson(httplib::Response &res, const json &payload, int status) const
    {
        res.status = status;
        res.set_content(payload.dump(), "application/json");
    }

    void HttpRideServer::createServerMethods()
    {
        server_->Get("/", [this](const httplib::Request &, httplib::Response &res)
                    {
                        respondJson(res,
                                    json{{"status", "ok"},
                                         {"service", "RideManager"},
                                         {"message", "Ride Manager HTTP interface is running"}});
                    });

        server_->Post("/rides/request", [this](const httplib::Request &req, httplib::Response &res)
                      {
                          auto payload = parseJsonBody(req, res);
                          if (payload.empty())
                          {
                              return;
                          }

                          auto result = routeHandler_->handleRideRequest(payload);
                          int statusCode = result.value("http_status", 500);
                          result.erase("http_status");
                          respondJson(res, result, statusCode);
                      });

        server_->Post(R"(/rides/(\w+)/status)", [this](const httplib::Request &req, httplib::Response &res)
                      {
                          auto payload = parseJsonBody(req, res);
                          if (payload.empty())
                          {
                              return;
                          }

                          const std::string rideId = req.matches[1];
                          auto result = routeHandler_->handleRideStatusUpdate(rideId, payload);
                          int statusCode = result.value("http_status", 500);
                          result.erase("http_status");
                          respondJson(res, result, statusCode);
                      });

        server_->Post(R"(/drivers/(\w+)/status)", [this](const httplib::Request &req, httplib::Response &res)
                      {
                          auto payload = parseJsonBody(req, res);
                          if (payload.empty())
                          {
                              return;
                          }

                          const std::string driverId = req.matches[1];
                          auto result = routeHandler_->handleDriverStatusUpdate(driverId, payload);
                          int statusCode = result.value("http_status", 500);
                          result.erase("http_status");
                          respondJson(res, result, statusCode);
                      });

        server_->Get(R"(/rides/(\w+))", [this](const httplib::Request &req, httplib::Response &res)
                    {
                        const std::string rideId = req.matches[1];
                        auto result = routeHandler_->handleGetRide(rideId);
                        int statusCode = result.value("http_status", 500);
                        result.erase("http_status");
                        respondJson(res, result, statusCode);
                    });

        server_->Get(R"(/rides/user/(\w+))", [this](const httplib::Request &req, httplib::Response &res)
                    {
                        const std::string userId = req.matches[1];
                        auto result = routeHandler_->handleGetRidesForUser(userId);
                        int statusCode = result.value("http_status", 500);
                        result.erase("http_status");
                        respondJson(res, result, statusCode);
                    });

        server_->Get(R"(/rides/driver/(\w+))", [this](const httplib::Request &req, httplib::Response &res)
                    {
                        const std::string driverId = req.matches[1];
                        auto result = routeHandler_->handleGetRidesForDriver(driverId);
                        int statusCode = result.value("http_status", 500);
                        result.erase("http_status");
                        respondJson(res, result, statusCode);
                    });

        server_->Get(R"(/drivers/(\w+)/profile)", [this](const httplib::Request &req, httplib::Response &res)
                    {
                        const std::string driverId = req.matches[1];
                        auto result = routeHandler_->handleGetDriverProfile(driverId);
                        int statusCode = result.value("http_status", 500);
                        result.erase("http_status");
                        respondJson(res, result, statusCode);
                    });
    }
}
