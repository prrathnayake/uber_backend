#include <iostream>
#include <filesystem>

#include "../../include/database/database.h"
#include <database/index.h>

namespace fs = std::filesystem;

uber_backend::uber_database::uber_database() : database(std::make_unique<database::MySQLDatabase>()), fileLogger(std::make_unique<utils::FileLogger>()) {};

uber_backend::uber_database::~uber_database() {};

void uber_backend::uber_database::initalizeDatabase()
{
    fileLogger->logMeta(utils::FileLogger::INFO, "Connecting to database...", __FILE__, __LINE__, __func__);

    if (!database->connect(host, user, password, "", port))
    {
        fileLogger->logMeta(utils::FileLogger::ERROR, "Database connection failed.", __FILE__, __LINE__, __func__);
        return;
    }

    fileLogger->logMeta(utils::FileLogger::INFO, "Connected to database.", __FILE__, __LINE__, __func__);

    // ✅ Build relative path safely
    fs::path sourcePath(__FILE__);
    fs::path scriptPath = sourcePath.parent_path() / "../../sql_scripts/database_init.sql";
    scriptPath = scriptPath.lexically_normal(); // Clean up ../ in path

    // ✅ Check if the file exists
    if (!fs::exists(scriptPath))
    {
        fileLogger->logMeta(utils::FileLogger::ERROR, "SQL script file not found at: " + scriptPath.string(), __FILE__, __LINE__, __func__);
        return;
    }

    // ✅ Run the script
    fileLogger->logMeta(utils::FileLogger::INFO, "Running SQL script: " + scriptPath.string(), __FILE__, __LINE__, __func__);
    database->runSqlScript(scriptPath.string());

    fileLogger->logMeta(utils::FileLogger::INFO, "Database initialized successfully.", __FILE__, __LINE__, __func__);
};