#include <iostream>
#include <filesystem>

#include "../../include/database/database.h"
#include <database/index.h>
#include <utils/log/singletonLogger.h>  // Make sure this include points to your singleton logger header

namespace fs = std::filesystem;

uber_backend::uber_database::uber_database()
    : database(std::make_unique<database::MySQLDatabase>())
{
    // No local fileLogger instance anymore
    // Just ensure singleton logger is initialized (optional)
    auto& logger = utils::SingletonLogger::instance();  
};

uber_backend::uber_database::~uber_database() {};

void uber_backend::uber_database::initalizeDatabase()
{
    auto& logger = utils::SingletonLogger::instance();

    logger.logMeta(utils::SingletonLogger::INFO, "Connecting to database...", __FILE__, __LINE__, __func__);

    if (!database->connect(host, user, password, "", port))
    {
        logger.logMeta(utils::SingletonLogger::ERROR, "Database connection failed.", __FILE__, __LINE__, __func__);
        return;
    }

    logger.logMeta(utils::SingletonLogger::INFO, "Connected to database.", __FILE__, __LINE__, __func__);

    // Build relative path safely
    fs::path sourcePath(__FILE__);
    fs::path scriptPath = sourcePath.parent_path() / "../../sql_scripts/database_init.sql";
    scriptPath = scriptPath.lexically_normal(); // Clean up ../ in path

    // Check if the file exists
    if (!fs::exists(scriptPath))
    {
        logger.logMeta(utils::SingletonLogger::ERROR, "SQL script file not found at: " + scriptPath.string(), __FILE__, __LINE__, __func__);
        return;
    }

    // Run the script
    logger.logMeta(utils::SingletonLogger::INFO, "Running SQL script: " + scriptPath.string(), __FILE__, __LINE__, __func__);
    database->runSqlScript(scriptPath.string());

    logger.logMeta(utils::SingletonLogger::INFO, "Database initialized successfully.", __FILE__, __LINE__, __func__);
};
