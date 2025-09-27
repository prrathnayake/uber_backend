#include <netdb.h>
#include <vector>

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

    auto canResolveHost = [](const std::string &host) {
        addrinfo hints{};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        addrinfo *result = nullptr;
        const int rc = getaddrinfo(host.c_str(), nullptr, &hints, &result);
        if (result)
        {
            freeaddrinfo(result);
        }
        return rc == 0;
    };

    std::string selectedHost = host_;
    if (!canResolveHost(selectedHost))
    {
        logger_.logMeta(SingletonLogger::WARNING,
                        "Configured host '" + selectedHost + "' is not resolvable. Falling back to loopback interfaces.",
                        __FILE__,
                        __LINE__,
                        __func__);

        const std::vector<std::string> fallbackHosts = {"127.0.0.1", "0.0.0.0"};
        bool resolved = false;
        for (const auto &candidate : fallbackHosts)
        {
            if (candidate == selectedHost)
            {
                continue;
            }

            if (canResolveHost(candidate))
            {
                selectedHost = candidate;
                resolved = true;
                break;
            }
        }

        if (!resolved)
        {
            logger_.logMeta(SingletonLogger::ERROR,
                            "Unable to resolve any fallback host for HTTP server '" + serverName_ + "'.",
                            __FILE__,
                            __LINE__,
                            __func__);
            is_running_ = false;
            return;
        }

        logger_.logMeta(SingletonLogger::INFO,
                        "HTTP server '" + serverName_ + "' will listen on fallback host '" + selectedHost + "'.",
                        __FILE__,
                        __LINE__,
                        __func__);
        host_ = selectedHost;
    }

    logger_.logMeta(SingletonLogger::INFO,
                    "Binding HTTP server '" + serverName_ + "' to " + selectedHost + ':' + std::to_string(port_),
                    __FILE__,
                    __LINE__,
                    __func__);

    if (!server_->listen(selectedHost.c_str(), port_))
    {
        logger_.logMeta(SingletonLogger::ERROR,
                        "HTTP server '" + serverName_ + "' failed to bind to " + selectedHost + ':' + std::to_string(port_),
                        __FILE__,
                        __LINE__,
                        __func__);
    }

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
