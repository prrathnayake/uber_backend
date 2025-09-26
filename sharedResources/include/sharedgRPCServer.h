#pragma once

#include <atomic>
#include <mutex>
#include <string>

#include <utils/index.h>

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
        std::string serverAddress_;
        mutable std::mutex mutex_;
        std::atomic<bool> running_{false};
    };
}
