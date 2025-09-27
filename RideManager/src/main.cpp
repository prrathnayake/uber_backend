#include <csignal>
#include <chrono>
#include <filesystem>
#include <thread>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace UberBackend;
using namespace utils;

namespace
{
    SingletonLogger &logger = SingletonLogger::instance();
    std::unique_ptr<RideServer> server;

    volatile std::sig_atomic_t shuttingDown = 0;
    volatile std::sig_atomic_t lastSignal = 0;

    void handleShutdownSignal(int signal)
    {
        shuttingDown = 1;
        lastSignal = signal;
    }

    void startApplication()
    {
        logger.logMeta(SingletonLogger::INFO, "Starting Uber Ride Manager Server...", __FILE__, __LINE__, __func__);

        const auto sourceDir = std::filesystem::path(__FILE__).parent_path();
        const auto projectRoot = (sourceDir / "../../").lexically_normal();
        const auto envPath = (projectRoot / ".env").lexically_normal();
        ConfigManager::instance().loadFromFile(envPath.string());

        server = std::make_unique<RideServer>("RideManager",
                                              UberUtils::CONFIG::getRideManagerDatabaseHost(),
                                              UberUtils::CONFIG::getRideManagerUsername(),
                                              UberUtils::CONFIG::getRideManagerPassword(),
                                              UberUtils::CONFIG::getRideManagerDatabase(),
                                              UberUtils::CONFIG::getRideManagerDatabasePort());

        const auto initScript = (projectRoot / "RideManager/sql_scripts/database_init.sql").lexically_normal();
        server->initiateDatabase(initScript.string());
        server->createHttpServers();
        server->startHttpServers();
        server->startConsumers();
    }

    void stopApplication()
    {
        logger.logMeta(SingletonLogger::INFO, "Stopping Uber Ride Manager Server...", __FILE__, __LINE__, __func__);
        if (!server)
        {
            return;
        }

        server->stopConsumers();
        server->stopHttpServers();
    }
}

int main()
{
    std::signal(SIGINT, handleShutdownSignal);
    std::signal(SIGTERM, handleShutdownSignal);

    startApplication();

    while (shuttingDown == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (lastSignal != 0)
    {
        logger.logMeta(SingletonLogger::INFO,
                       "Received shutdown signal: " + std::to_string(lastSignal),
                       __FILE__,
                       __LINE__,
                       __func__);
    }

    stopApplication();
    return 0;
}
