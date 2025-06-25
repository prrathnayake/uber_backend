#include <iostream>
#include <vector>
#include <httplib.h>

#include <utils/index.h>
#include "../../../include/services/httpService/httpService.h"

using namespace utils;
using namespace uber_backend;

HttpService::HttpService()
    : logger_(SingletonLogger::instance()) {}

HttpService::~HttpService()
{
    // Optionally ensure all servers stop when the service is destroyed
    stopServers();
}

void HttpService::createServer()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating a lightweight HTTP server.", __FILE__, __LINE__, __func__);

    auto simpleServer = std::make_unique<uber_backend::HttpServer>("localhost", 8081);

    // Add a simple health-check route
    simpleServer->Get("/ping", [](const httplib::Request &req, httplib::Response &res)
                      { res.set_content("pong", "text/plain"); });

    logger_.logMeta(SingletonLogger::INFO, "Added lightweight server to vector.", __FILE__, __LINE__, __func__);

    servers.push_back(std::move(simpleServer));
}

void HttpService::initiateServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Initiating servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers)
    {
        server->start();
    }

    logger_.logMeta(SingletonLogger::INFO, "All servers have been initiated.", __FILE__, __LINE__, __func__);
}

void HttpService::stopServers()
{
    logger_.logMeta(SingletonLogger::INFO, "Stopping servers...", __FILE__, __LINE__, __func__);

    for (auto &server : servers)
    {
        logger_.logMeta(SingletonLogger::INFO, "stoped", __FILE__, __LINE__, __func__);

        server->stop();
    }

    logger_.logMeta(SingletonLogger::INFO, "All servers stopped.", __FILE__, __LINE__, __func__);
}
