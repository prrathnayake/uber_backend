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
    const std::filesystem::path kLogPath = (kProjectRoot / "log/user_manager.log").lexically_normal();

    SingletonLogger &logger = SingletonLogger::instance(kLogPath.string());

    volatile std::sig_atomic_t shuttingDown = 0;
    volatile std::sig_atomic_t lastSignal = 0;

    void handleShutdownSignal(int signal)
    {
        shuttingDown = 1;
        lastSignal = signal;
    }

    std::unique_ptr<Server> server;
}

// Staring the application
void startApplication()
{
    logger.logMeta(SingletonLogger::INFO, "Starting Uber User Manager Server......", __FILE__, __LINE__, __func__);

    const auto envPath = (kProjectRoot / ".env").lexically_normal();
    UberBackend::ConfigManager::instance().loadFromFile(envPath.string());

    logger.logMeta(SingletonLogger::DEBUG, "creating server instance for user Manager", __FILE__, __LINE__, __func__);

    server = std::make_unique<Server>("UserManager",
                                      UberUtils::CONFIG::getUserManagerDatabaseHost(),
                                      UberUtils::CONFIG::getUserManagerUsername(),
                                      UberUtils::CONFIG::getUserManagerPassword(),
                                      UberUtils::CONFIG::getUserManagerDatabase(),
                                      UberUtils::CONFIG::getUserManagerDatabasePort());

    const auto initScript = (kProjectRoot / "UserManager/sql_scripts/database_init.sql").lexically_normal();

    server->initiateDatabase(initScript.string());
    server->createHttpServers();
    logger.logMeta(SingletonLogger::DEBUG, "run : server->startHttpServers();", __FILE__, __LINE__, __func__);
    server->startHttpServers();
}

/// stopping the application
void stopApplication()
{
    logger.logMeta(SingletonLogger::INFO, "Stoping Uber User Manager Server......", __FILE__, __LINE__, __func__);
    if (!server)
    {
        logger.logMeta(SingletonLogger::WARNING, "Server instance was not initialised before stop call", __FILE__, __LINE__, __func__);
        return;
    }

    server->stopHttpServers();

    const auto dropScript = (kProjectRoot / "UserManager/sql_scripts/database_del.sql").lexically_normal();
    logger.logMeta(SingletonLogger::INFO, "distroying server instance for user Manager", __FILE__, __LINE__, __func__);

    // Distorying the database - this will drop all the tables and data just for debugging purposes
    server->distoryDatabase(dropScript.string());
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
