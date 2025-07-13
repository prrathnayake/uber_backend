#include <iostream>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

// Getting instance of the Singleton logger
auto &logger_ = SingletonLogger::instance("log/UserServerLog.txt");

// Creating a unique pointer for the Server instance
std::unique_ptr<Server> server_ = std::make_unique<Server>("UserManager",
                                                           UberUtils::CONFIG::USER_MANAGER_HOST,
                                                           UberUtils::CONFIG::USER_MANAGER_USERNAME,
                                                           UberUtils::CONFIG::USER_MANAGER_PASSWORD,
                                                           UberUtils::CONFIG::USER_MANAGER_DATABASE_NAME,
                                                           UberUtils::CONFIG::USER_MANAGER_DATABASE_PORT);

// Staring the application
void startApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Starting Uber User Manager Server......", __FILE__, __LINE__, __func__);
    std::string path = "../../UserManager/sql_scripts/database_init.sql";

    logger_.logMeta(SingletonLogger::DEBUG, "creating server instance for user Manager", __FILE__, __LINE__, __func__);

    server_->initiateDatabase(path);
    server_->createHttpServers();
    logger_.logMeta(SingletonLogger::DEBUG, "run : server_->startHttpServers();", __FILE__, __LINE__, __func__);
    server_->startHttpServers();
}

/// stopping the application
void stopApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Stoping Uber User Manager Server......", __FILE__, __LINE__, __func__);
    server_->stopHttpServers();

    std::string path = "../../UserManager/sql_scripts/database_del.sql";
    logger_.logMeta(SingletonLogger::INFO, "distroying server instance for user Manager", __FILE__, __LINE__, __func__);

    // Distorying the database - this will drop all the tables and data just for debugging purposes
    server_->distoryDatabase(path);
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