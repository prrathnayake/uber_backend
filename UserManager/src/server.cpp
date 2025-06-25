#include <iostream>

#include <utils/index.h>
#include "../include/server.h"

using namespace utils;
using namespace uber_backend;

Server::Server() : logger_(SingletonLogger::instance()) // initialize reference once
{
    thread_pool_ = std::make_unique<ThreadPool>(64);
    logger_.logMeta(SingletonLogger::INFO, "Server initialized", __FILE__, __LINE__, __func__);
}

void Server::initiateDatabase()
{
    logger_.logMeta(SingletonLogger::INFO, "Creating database instance.", __FILE__, __LINE__, __func__);

    // Now create database instance passing those variables
    database_ = std::make_unique<uber_database>(host, user, password, databaseName, port);

    database_->runSQLScript("../../sql_scripts/database_init.sql");
}
