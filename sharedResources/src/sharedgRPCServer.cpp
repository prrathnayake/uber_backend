#include "../include/sharedgRPCServer.h"

namespace UberBackend
{
    SharedgPRCServer::SharedgPRCServer(const std::string &server_address)
        : logger_(utils::SingletonLogger::instance()),
          serverAddress_(server_address)
    {
    }

    SharedgPRCServer::~SharedgPRCServer()
    {
        Shutdown();
    }

    void SharedgPRCServer::Run()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_)
        {
            logger_.logMeta(utils::SingletonLogger::WARNING,
                            "Attempted to start simulated gRPC server while already running",
                            __FILE__,
                            __LINE__,
                            __func__);
            return;
        }

        running_ = true;
        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Simulated gRPC server started on " + serverAddress_,
                        __FILE__,
                        __LINE__,
                        __func__);
    }

    void SharedgPRCServer::Shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_)
        {
            return;
        }

        running_ = false;
        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Simulated gRPC server shut down",
                        __FILE__,
                        __LINE__,
                        __func__);
    }

    bool SharedgPRCServer::isRunning() const
    {
        return running_.load();
    }
}
