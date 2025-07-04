#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>
#include <utils/index.h>

using namespace utils;

namespace UberBackend
{
    class SharedgPRCServer
    {
    public:
        SharedgPRCServer(const std::string &server_address);
        ~SharedgPRCServer();

        void Run();

    protected:
        SingletonLogger &logger_;
        std::string serverAddress;
        std::unique_ptr<grpc::Server> grpcServer;
    };
}
