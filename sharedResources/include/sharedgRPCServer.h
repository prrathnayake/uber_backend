#pragma once

#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>
#include <utils/index.h>

#include "../../proto/location.grpc.pb.h"
#include "../../proto/location.pb.h"

namespace UberBackend
{
    class SharedgPRCServer
    {
    public:
        explicit SharedgPRCServer(const std::string &server_address);
        ~SharedgPRCServer();

        void Run();

    protected:
        utils::SingletonLogger &logger_;
        std::string serverAddress;
        std::unique_ptr<grpc::Server> grpcServer;
    };
}
