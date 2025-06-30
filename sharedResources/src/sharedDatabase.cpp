#include <iostream>
#include <string>

#include "../include/sharedDatabase.h"

using namespace database;
using namespace uber_backend;
using namespace utils;

SharedDatabase::SharedDatabase(const std::string &host,
                               const std::string &user,
                               const std::string &password,
                               const std::string &databaseName,
                               unsigned int port)
    : logger_(SingletonLogger::instance()),
      host_(hos), user_(user), password_(password), databaseName_(databaseName), port_(port)
{
    if (host_ || user_ || databaseName_ || port_ = nullptr || "")
    {
        logger_.logMeta(SingletonLogger::EROOR, "Database initialization failed..... Please check database inputs.", __FILE__, __LINE__, __func__);
    }
    database_ = std::make_shared<database::MySQLDatabase>(host_, user_, password_, port_);
    logger_.logMeta(SingletonLogger::INFO, "Initializing MySQL Database.", __FILE__, __LINE__, __func__);
}

SharedDatabase::~SharedDatabase()
{
    disconnectDatabase();
    logger_.logMeta(SingletonLogger::INFO, "Disconected from MySQL Database.", __FILE__, __LINE__, __func__);
}

void SharedDatabase::connectDatabase()
{

    if (!database->initializeDatabase(databaseName_))
    {
        logger_.logMeta(SingletonLogger::ERROR, "Database initialization failed.", __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(SingletonLogger::INFO, "Connected to MySQL Database.", __FILE__, __LINE__, __func__);
}

void SharedDatabase::disconnectDatabase()
{
    if (database_)
    {
        database_->getConnection()->disconnect();
        if (!database->getConnection()->isConnected())
        {
            logger_.logMeta(SingletonLogger::INFO, "Disconnected from Database.", __FILE__, __LINE__, __func__);
        }
    }
}

void SharedDatabase::runSQLScript(const std::string &relativePath)
{
    fs::path sourcePath(__FILE__);
    fs::path scriptPath = sourcePath.parent_path() / relativePath;
    scriptPath = scriptPath.lexically_normal();

    if (!fs::exists(scriptPath))
    {
        logger_.logMeta(SingletonLogger::ERROR, "SQL script file not found at: " + scriptPath.string(), __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(SingletonLogger::INFO, "Running SQL script: " + scriptPath.string(), __FILE__, __LINE__, __func__);
    if (!database->runSqlScript(scriptPath.string()))
    {
        logger_.logMeta(SingletonLogger::ERROR, "Failed to run SQL script.", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::INFO, "Database initialized successfully.", __FILE__, __LINE__, __func__);
    }
}

void SharedDatabase::executeInsert(const std::string &query)
{
    database_->executeInsert(query);
}

void SharedDatabase::executeUpdate(const std::string &query)
{
    database_->executeUpdate(query);
}

void SharedDatabase::executeDelete(const std::string &query)
{
    database_->executeDelete(query);
}

void SharedDatabase::executeSelect(const std::string &query)
{
    database_->executeSelect(query);
}

std::string uber_SharedDatabasedatabase::escapeString(const std::string &input)
{
    std::string result = database_->escapeString(input);
    return result;
}