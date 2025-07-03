#include <utils/index.h>

#include "../include/sharedHTTPServer.h"

using namespace UberBackend;
using namespace utils;

SharedHttpServer::SharedHttpServer(const std::string &serverName, const std::string &host, int port, std::shared_ptr<SharedDatabase> db)
    : host_(host), port_(port), is_running_(false),
      server_(std::make_unique<httplib::Server>()),
      logger_(SingletonLogger::instance()),
      database_(db),
      serverName_(serverName)
{
    if (serverName_.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP server name.", __FILE__, __LINE__, __func__);
    }
    if (port_ == 0)
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP server port.", __FILE__, __LINE__, __func__);
    }
    if (host_.empty())
    {
        logger_.logMeta(SingletonLogger::ERROR, "No HTTP server host.", __FILE__, __LINE__, __func__);
    }
}

SharedHttpServer::~SharedHttpServer()
{
    stop();
}

void SharedHttpServer::start()
{
    logger_.logMeta(SingletonLogger::INFO, "Starting HTTP server : " + serverName_, __FILE__, __LINE__, __func__);

    if (is_running_)
        return;

    is_running_ = true;

    server_->listen(host_.c_str(), port_);

    is_running_ = false;
}

void SharedHttpServer::stop()
{
    logger_.logMeta(SingletonLogger::INFO, "Stoping HTTP server : " + serverName_, __FILE__, __LINE__, __func__);

    if (!is_running_)
        return;

    server_->stop();

    is_running_ = false;
}
