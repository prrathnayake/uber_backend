#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

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
        void Shutdown();
        [[nodiscard]] bool isRunning() const;

    protected:
        utils::SingletonLogger &logger_;
        std::string serverAddress;
        std::unique_ptr<grpc::Server> grpcServer;
        mutable std::mutex mutex_;
        std::atomic<bool> running_{false};
    };
}
