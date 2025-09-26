#include <filesystem>
#include <iostream>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

// Getting instance of the Singleton logger
auto &logger_ = SingletonLogger::instance();

// Creating a unique pointer for the Server instance
std::unique_ptr<Server> server_;

// Staring the application
void startApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Starting Uber User Manager Server......", __FILE__, __LINE__, __func__);
    const auto sourceDir = std::filesystem::path(__FILE__).parent_path();
    const auto projectRoot = (sourceDir / "../../").lexically_normal();

    const auto envPath = (projectRoot / ".env").lexically_normal();
    UberBackend::ConfigManager::instance().loadFromFile(envPath.string());

    logger_.logMeta(SingletonLogger::DEBUG, "creating server instance for user Manager", __FILE__, __LINE__, __func__);

    server_ = std::make_unique<Server>("UserManager",
                                       UberUtils::CONFIG::getUserManagerHost(),
                                       UberUtils::CONFIG::getUserManagerUsername(),
                                       UberUtils::CONFIG::getUserManagerPassword(),
                                       UberUtils::CONFIG::getUserManagerDatabase(),
                                       UberUtils::CONFIG::getUserManagerDatabasePort());

    const auto initScript = (projectRoot / "UserManager/sql_scripts/database_init.sql").lexically_normal();

    server_->initiateDatabase(initScript.string());
    server_->createHttpServers();
    logger_.logMeta(SingletonLogger::DEBUG, "run : server_->startHttpServers();", __FILE__, __LINE__, __func__);
    server_->startHttpServers();
}

/// stopping the application
void stopApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Stoping Uber User Manager Server......", __FILE__, __LINE__, __func__);
    if (!server_)
    {
        logger_.logMeta(SingletonLogger::WARNING, "Server instance was not initialised before stop call", __FILE__, __LINE__, __func__);
        return;
    }

    server_->stopHttpServers();

    const auto sourceDir = std::filesystem::path(__FILE__).parent_path();
    const auto projectRoot = (sourceDir / "../../").lexically_normal();
    const auto dropScript = (projectRoot / "UserManager/sql_scripts/database_del.sql").lexically_normal();
    logger_.logMeta(SingletonLogger::INFO, "distroying server instance for user Manager", __FILE__, __LINE__, __func__);

    // Distorying the database - this will drop all the tables and data just for debugging purposes
    server_->distoryDatabase(dropScript.string());
}

int main()
{
    startApplication();
    // Wait for user input to stop the server just for debugging purposes
    std::cout << "Press Enter to stop server...\n";
    std::cin.get();
    stopApplication();
    return 0;
}