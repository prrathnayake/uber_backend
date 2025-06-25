#include "../include/services/httpService/httpServer.h"

using namespace uber_backend;

HttpServer::HttpServer(const std::string &host, int port)
    : host_(host), port_(port), is_running_(false), server_(std::make_unique<httplib::Server>())
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

    // This call blocks the current thread until the server stops or is stopped.
    server_->listen(host_.c_str(), port_);

    is_running_ = false;
}

void HttpServer::stop()
{
    if (!is_running_)
        return;

    server_->stop();
    // No thread join needed since no thread was used

    is_running_ = false;
}

void HttpServer::Get(const std::string &path, httplib::Server::Handler handler)
{
    server_->Get(path.c_str(), handler);
}
