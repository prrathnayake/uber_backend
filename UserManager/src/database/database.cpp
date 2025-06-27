#include "../../include/database/database.h"
#include <filesystem>
#include <utils/index.h>

namespace fs = std::filesystem;

using namespace uber_backend;

uber_database::uber_database(const std::string &host_,
                             const std::string &user_,
                             const std::string &password_,
                             const std::string &databaseName_,
                             unsigned int port_)
    : logger_(utils::SingletonLogger::instance()),
      host(host_), user(user_), password(password_), databaseName(databaseName_), port(port_)
{
    database = std::make_shared<database::MySQLDatabase>(host, user, password, port);
}

uber_database::~uber_database()
{
    disconnectDatabase();
}

void uber_database::connectDatabase()
{
    logger_.logMeta(utils::SingletonLogger::INFO, "Connecting to database... Database: " + databaseName, __FILE__, __LINE__, __func__);

    if (!database->initializeDatabase(databaseName))
    {
        logger_.logMeta(utils::SingletonLogger::ERROR, "Database initialization failed.", __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(utils::SingletonLogger::INFO, "Connected to database.", __FILE__, __LINE__, __func__);
}

void uber_database::disconnectDatabase()
{
    logger_.logMeta(utils::SingletonLogger::INFO, "Disconnecting from database...", __FILE__, __LINE__, __func__);

    if (database)
    {
        database->getConnection()->disconnect();
        if (!database->getConnection()->isConnected())
        {
            logger_.logMeta(utils::SingletonLogger::INFO, "Disconnected from database.", __FILE__, __LINE__, __func__);
        }
    }
}

void uber_database::runSQLScript(const std::string &relativePath)
{
    fs::path sourcePath(__FILE__);
    fs::path scriptPath = sourcePath.parent_path() / relativePath;
    scriptPath = scriptPath.lexically_normal();

    if (!fs::exists(scriptPath))
    {
        logger_.logMeta(utils::SingletonLogger::ERROR, "SQL script file not found at: " + scriptPath.string(), __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(utils::SingletonLogger::INFO, "Running SQL script: " + scriptPath.string(), __FILE__, __LINE__, __func__);
    if (!database->runSqlScript(scriptPath.string()))
    {
        logger_.logMeta(utils::SingletonLogger::ERROR, "Failed to run SQL script.", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(utils::SingletonLogger::INFO, "Database initialized successfully.", __FILE__, __LINE__, __func__);
    }
}

void uber_database::executeInsert(const std::string &query)
{
    database->executeInsert(query);
}

void uber_database::executeUpdate(const std::string &query)
{
    database->executeUpdate(query);
}

void uber_database::executeDelete(const std::string &query)
{
    database->executeDelete(query);
}

void uber_database::executeSelect(const std::string &query)
{
    database->executeSelect(query);
}

std::string uber_database::escapeString(const std::string &input)
{
    std::string result = database->escapeString(input);
    return result;
}
