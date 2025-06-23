#include <iostream>

#include "./database.h"
#include <database/index.h>

uber_backend::uber_database::uber_database() {};

uber_backend::uber_database::~uber_database() {};

void uber_backend::uber_database::initalizeDatabase()
{
    database::MySQLDatabase database;

    fileLogger->logMeta(utils::FileLogger::INFO, "connecting Database........", __FILE__, __LINE__, __func__);


    if (!database.connect(
            host,
            user,
            password,
            "",
            port))
    {
        fileLogger->logMeta(utils::FileLogger::ERROR, "Database connection failed.", __FILE__, __LINE__, __func__);
        return;
    }

    // execute initial SQL script to initialize the database for the application
    fileLogger->logMeta(utils::FileLogger::INFO, "Initializing Database........", __FILE__, __LINE__, __func__);
    std::string pathToScript = "/home/pasan/PROJECTS/cpp/uber/sql_scripts/database._init.sql";

    database.runSqlScript(pathToScript);
    fileLogger->logMeta(utils::FileLogger::INFO, "Database has initializeed.", __FILE__, __LINE__, __func__);

    bool isConnect = database.isConnected();
    fileLogger->logMeta(utils::FileLogger::INFO, "Database connected.", __FILE__, __LINE__, __func__);
};
