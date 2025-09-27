#include <filesystem>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace UberBackend;
using namespace utils;

namespace
{
    SingletonLogger &logger = SingletonLogger::instance();
    std::unique_ptr<RideServer> server;

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
    startApplication();
    logger.logMeta(SingletonLogger::INFO, "RideManager service running. Press Enter to exit...", __FILE__, __LINE__, __func__);
    std::cin.get();
    stopApplication();
    return 0;
}
