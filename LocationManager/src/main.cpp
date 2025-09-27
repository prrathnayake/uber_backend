#include <csignal>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

namespace
{
    const std::filesystem::path kSourceDir = std::filesystem::path(__FILE__).parent_path();
    const std::filesystem::path kProjectRoot = (kSourceDir / "../../").lexically_normal();
    const std::filesystem::path kLogPath = (kProjectRoot / "log/location_manager.log").lexically_normal();

    SingletonLogger &logger = SingletonLogger::instance(kLogPath.string());
    std::unique_ptr<Server> server;

    volatile std::sig_atomic_t shuttingDown = 0;
    volatile std::sig_atomic_t lastSignal = 0;
}

namespace
{
    void handleShutdownSignal(int signal)
    {
        shuttingDown = 1;
        lastSignal = signal;
    }
}

void startApplication()
{
    logger.logMeta(SingletonLogger::INFO, "Starting Uber Location Manager Server......", __FILE__, __LINE__, __func__);

    const auto envPath = (kProjectRoot / ".env").lexically_normal();

    ConfigManager::instance().loadFromFile(envPath.string());

    server = std::make_unique<Server>("LocationManager",
                                      UberUtils::CONFIG::getLocationManagerDatabaseHost(),
                                      UberUtils::CONFIG::getLocationManagerUsername(),
                                      UberUtils::CONFIG::getLocationManagerPassword(),
                                      UberUtils::CONFIG::getLocationManagerDatabase(),
                                      UberUtils::CONFIG::getLocationManagerDatabasePort());

    const auto initScript = (kProjectRoot / "LocationManager/sql_scripts/database_init.sql").lexically_normal();
    server->initiateDatabase(initScript.string());
    server->createHttpServers();
    server->startHttpServers();
    logger.logMeta(SingletonLogger::DEBUG, "run : server->startConsumers();", __FILE__, __LINE__, __func__);
    server->startConsumers();
}

void stopApplication()
{
    logger.logMeta(SingletonLogger::INFO, "Stopping Uber Location Manager Server......", __FILE__, __LINE__, __func__);

    if (!server)
    {
        return;
    }

    server->stopConsumers();
    server->stopGrpcServer();
    server->stopHttpServers();
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
