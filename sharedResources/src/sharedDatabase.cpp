#include <iostream>
#include <string>
#include <filesystem>

#include "../include/sharedDatabase.h"

using namespace database;
using namespace UberBackend;
using namespace utils;
namespace fs = std::filesystem;

SharedDatabase::SharedDatabase(const std::string &host,
                               const std::string &user,
                               const std::string &password,
                               const std::string &databaseName,
                               unsigned int port)
    : logger_(SingletonLogger::instance()),
      host_(host), user_(user), password_(password), databaseName_(databaseName), port_(port)
{
    if (host_.empty() || user_.empty() || databaseName_.empty() || port_ == 0)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Database initialization failed... Please check database inputs.", __FILE__, __LINE__, __func__);
    }
    database_ = std::make_shared<database::MySQLDatabase>(host_, user_, password_, port_, databaseName_);
    logger_.logMeta(SingletonLogger::INFO, "Initializing MySQL Database.", __FILE__, __LINE__, __func__);
}

SharedDatabase::~SharedDatabase()
{

    logger_.logMeta(SingletonLogger::INFO, "Disconected from MySQL Database.", __FILE__, __LINE__, __func__);
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
    if (!database_->runSqlScript(scriptPath.string()))
    {
        logger_.logMeta(SingletonLogger::ERROR, "Failed to run SQL script.", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::INFO, "SQL script executed successfully.", __FILE__, __LINE__, __func__);
    }
}

bool SharedDatabase::executeInsert(const std::string &query)
{
    if (database_->executeInsert(query))
    {
        logger_.logMeta(SingletonLogger::INFO, "Database executeInsert() successfully.", __FILE__, __LINE__, __func__);
        return true;
    }
    else
    {
        logger_.logMeta(SingletonLogger::INFO, "Database executeInsert() unsuccessfull.", __FILE__, __LINE__, __func__);
        return false;
    }
}

bool SharedDatabase::executeUpdate(const std::string &query)
{
    if (database_->executeUpdate(query))
    {
        logger_.logMeta(SingletonLogger::INFO, "Database executeUpdate() successfully.", __FILE__, __LINE__, __func__);
        return true;
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Database executeUpdate() unsuccessful.", __FILE__, __LINE__, __func__);
        return false;
    }
}

bool SharedDatabase::executeDelete(const std::string &query)
{
    if (database_->executeDelete(query))
    {
        logger_.logMeta(SingletonLogger::INFO, "Database executeDelete() successfully.", __FILE__, __LINE__, __func__);
        return true;
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Database executeDelete() unsuccessful.", __FILE__, __LINE__, __func__);
        return false;
    }
}

bool SharedDatabase::executeSelect(const std::string &query)
{
    if (database_->executeSelect(query))
    {
        logger_.logMeta(SingletonLogger::INFO, "Database executeSelect() successfully.", __FILE__, __LINE__, __func__);
        return true;
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Database executeSelect() unsuccessful.", __FILE__, __LINE__, __func__);
        return false;
    }
}

// This function is used to escape strings for SQL queries
// It is used to prevent SQL injection attacks
// It is used to escape special characters in strings
std::string SharedDatabase::escapeString(const std::string &input)
{
    logger_.logMeta(SingletonLogger::INFO, "Escaping: " + input, __FILE__, __LINE__, __func__);
    if (input.empty() || input == "NULL" || input == "null" || input == "''" || input == "\"\"" || input == "")
    {
        logger_.logMeta(SingletonLogger::ERROR, "Input string is empty.", __FILE__, __LINE__, __func__);
        return "";
    }
    std::string result = database_->escapeString(input);
    logger_.logMeta(SingletonLogger::INFO, "Database escapeString() successfully.", __FILE__, __LINE__, __func__);
    return result;
}

std::vector<std::map<std::string, std::string>> SharedDatabase::fetchRows(const std::string &query)
{
    auto rows = database_->fetchRows(query);
    if (!rows.empty())
    {
        logger_.logMeta(SingletonLogger::INFO, "Database fetchRows() returned rows successfully.", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::WARNING, "Database fetchRows() returned no rows.", __FILE__, __LINE__, __func__);
    }
    return rows;
}
