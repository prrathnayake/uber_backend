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
    database_ = std::make_shared<database::MySQLDatabase>(host_, user_, password_, port_);
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
        logger_.logMeta(SingletonLogger::INFO, "Database initialized successfully.", __FILE__, __LINE__, __func__);
    }
}

bool SharedDatabase::executeInsert(const std::string &query)
{
    logger_.logMeta(SingletonLogger::INFO, "Database executeInsert() successfully.", __FILE__, __LINE__, __func__);
    return database_->executeInsert(query);
}

bool SharedDatabase::executeUpdate(const std::string &query)
{
    logger_.logMeta(SingletonLogger::INFO, "Database executeUpdate() successfully.", __FILE__, __LINE__, __func__);
    return database_->executeUpdate(query);
}

bool SharedDatabase::executeDelete(const std::string &query)
{
    logger_.logMeta(SingletonLogger::INFO, "Database executeDelete() successfully.", __FILE__, __LINE__, __func__);
    return database_->executeDelete(query);
}

bool SharedDatabase::executeSelect(const std::string &query)
{
    logger_.logMeta(SingletonLogger::INFO, "Database executeSelect() successfully.", __FILE__, __LINE__, __func__);
    return database_->executeSelect(query);
}

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
    logger_.logMeta(SingletonLogger::INFO, "Database fetchRows() successfully.", __FILE__, __LINE__, __func__);
    return database_->fetchRows(query);
}
