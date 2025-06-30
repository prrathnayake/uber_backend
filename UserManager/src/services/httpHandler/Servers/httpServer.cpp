#include <utils/index.h>

#include "../include/sharedHTTPServer.h"

using namespace uber_backend;
using namespace utils;

HttpServer::HttpServer(const std::string &host, int port, std::shared_ptr<uber_database> db)
    : host_(host), port_(port), is_running_(false),
      server_(std::make_unique<httplib::Server>()),
      logger_(SingletonLogger::instance()),
      database_(db)
{
}

HttpServer::~HttpServer()
{
    stop();
}

void HttpServer::start()
{
    if (is_running_)
        return;

    is_running_ = true;
    server_->listen(host_.c_str(), port_);

    is_running_ = false;
}

void HttpServer::stop()
{
    if (!is_running_)
        return;

    server_->stop();

    is_running_ = false;
}
