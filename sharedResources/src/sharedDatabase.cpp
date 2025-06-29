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