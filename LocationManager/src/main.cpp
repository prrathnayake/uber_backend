#include <iostream>

#include "../include/server.h"
#include "../../sharedUtils/include/config.h"

using namespace utils;
using namespace UberBackend;

auto &logger_ = SingletonLogger::instance("log/LocationServerLog.txt");

std::unique_ptr<Server> server_ = std::make_unique<Server>("LocationManager",
                                                           UberUtils::CONFIG::LOCATION_MANAGER_HOST,
                                                           UberUtils::CONFIG::LOCATION_MANAGER_USERNAME,
                                                           UberUtils::CONFIG::LOCATION_MANAGER_PASSWORD,
                                                           UberUtils::CONFIG::LOCATION_MANAGER_DATABASE_NAME,
                                                           UberUtils::CONFIG::LOCATION_MANAGER_DATABASE_PORT);

void startApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Starting Uber Location Manager Server......", __FILE__, __LINE__, __func__);
    std::string path = "../../LocationManager/sql_scripts/database_init.sql";

    logger_.logMeta(SingletonLogger::DEBUG, "creating server instance for user Manager", __FILE__, __LINE__, __func__);

    server_->initiateDatabase(path);
    server_->createHttpServers();
    logger_.logMeta(SingletonLogger::DEBUG, "run : server_->startHttpServers();", __FILE__, __LINE__, __func__);
    server_->startHttpServers();
    logger_.logMeta(SingletonLogger::DEBUG, "run : server_->startConsumers();", __FILE__, __LINE__, __func__);
    server_->startConsumers();
}

void stopApplication()
{
    logger_.logMeta(SingletonLogger::INFO, "Stoping Uber Location Manager Server......", __FILE__, __LINE__, __func__);
    server_->stopHttpServers();
    server_->stopConsumers();
}

int main()
{
    startApplication();
    std::cout << "Press Enter to stop server...\n";
    std::cin.get();
    stopApplication();
    return 0;
}