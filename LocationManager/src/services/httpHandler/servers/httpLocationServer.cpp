#include "../include/services/httpHandler/servers/httpLocationServer.h"

using namespace UberBackend;
using namespace utils;

HttpLocationServer::HttpLocationServer(const std::string &name,
                                       const std::string &host,
                                       int port,
                                       std::shared_ptr<SharedDatabase> db)
    : SharedHttpServer(name, host, port, db), jwt_("your_super_secret_key")
{
    // routHandler_ = std::make_shared<RouteHandler>(db);
}

void HttpLocationServer::createServerMethods()
{
    server_->Post("/location/update", [this](const httplib::Request &req, httplib::Response &res)
                  {
    auto jsonData = Json::parse(req.body);
    // locationHandler_->handleLocationUpdate(jsonData);
    res.set_content(R"({"message": "Location updated"})", "application/json"); });

    server_->Get(R"(/location/(\w+))", [this](const httplib::Request &req, httplib::Response &res)
                 {
    std::string userId = req.matches[1];
    // auto location = locationHandler_->handleGetLocationByUserId(userId);
    res.set_content("location.dump()", "application/json"); });

    server_->Get("/location/nearby", [this](const httplib::Request &req, httplib::Response &res)
                 {
    if (req.has_param("lat") && req.has_param("lng") && req.has_param("role")) {
        double lat = std::stod(req.get_param_value("lat"));
        double lng = std::stod(req.get_param_value("lng"));
        std::string role = req.get_param_value("role");

        // auto results = locationHandler_->handleFindNearby(lat, lng, role);
        res.set_content("results.dump()", "application/json");
    } else {
        res.status = 400;
        res.set_content(R"({"error": "Missing query params"})", "application/json");
    } });

    server_->Delete(R"(/location/(\w+))", [this](const httplib::Request &req, httplib::Response &res)
                    {
    std::string userId = req.matches[1];
    bool success = true ;
    // locationHandler_->handleDeleteLocation(userId);
    if (success)
        res.set_content(R"({"message": "Deleted"})", "application/json");
    else {
        res.status = 404;
        res.set_content(R"({"error": "User not found"})", "application/json");
    } });
}
