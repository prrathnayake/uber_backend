#include <filesystem>
#include <memory>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

namespace
{
    SingletonLogger &logger = SingletonLogger::instance();
    std::unique_ptr<Server> server;
}

void startApplication()
{
    logger.logMeta(SingletonLogger::INFO, "Starting Uber Location Manager Server......", __FILE__, __LINE__, __func__);

    const auto sourceDir = std::filesystem::path(__FILE__).parent_path();
    const auto projectRoot = (sourceDir / "../../").lexically_normal();
    const auto envPath = (projectRoot / ".env").lexically_normal();

    ConfigManager::instance().loadFromFile(envPath.string());

    server = std::make_unique<Server>("LocationManager",
                                      UberUtils::CONFIG::getLocationManagerDatabaseHost(),
                                      UberUtils::CONFIG::getLocationManagerUsername(),
                                      UberUtils::CONFIG::getLocationManagerPassword(),
                                      UberUtils::CONFIG::getLocationManagerDatabase(),
                                      UberUtils::CONFIG::getLocationManagerDatabasePort());

    const auto initScript = (projectRoot / "LocationManager/sql_scripts/database_init.sql").lexically_normal();
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

    server->stopHttpServers();
    server->stopConsumers();
}

int main()
{
    startApplication();
    logger.logMeta(SingletonLogger::INFO, "Press Enter to stop server...", __FILE__, __LINE__, __func__);
    std::cin.get();
    stopApplication();
    return 0;
}
