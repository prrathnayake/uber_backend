#include "../../include/database/MySQLDatabase.h"

#include <fstream>

namespace database {

MySQLDatabase::MySQLDatabase(std::string host,
                             std::string user,
                             std::string password,
                             unsigned int port,
                             std::string databaseName)
    : host_(std::move(host)),
      user_(std::move(user)),
      password_(std::move(password)),
      port_(port),
      databaseName_(std::move(databaseName)),
      logger_(utils::SingletonLogger::instance())
{
    logger_.logMeta(utils::SingletonLogger::INFO,
                    "Initialised stub MySQLDatabase for host=" + host_ + ":" + std::to_string(port_) +
                        " db=" + databaseName_,
                    __FILE__,
                    __LINE__,
                    __func__);
}

bool MySQLDatabase::runSqlScript(const std::string &path)
{
    std::ifstream script(path);
    const bool ok = script.good();
    logger_.logMeta(ok ? utils::SingletonLogger::INFO : utils::SingletonLogger::WARNING,
                    ok ? "Processed SQL script at " + path : "Failed to open SQL script at " + path,
                    __FILE__,
                    __LINE__,
                    __func__);
    return ok;
}

bool MySQLDatabase::executeInsert(const std::string &query)
{
    logger_.logMeta(utils::SingletonLogger::DEBUG, "Stub INSERT query: " + query, __FILE__, __LINE__, __func__);
    return !query.empty();
}

bool MySQLDatabase::executeUpdate(const std::string &query)
{
    logger_.logMeta(utils::SingletonLogger::DEBUG, "Stub UPDATE query: " + query, __FILE__, __LINE__, __func__);
    return !query.empty();
}

bool MySQLDatabase::executeDelete(const std::string &query)
{
    logger_.logMeta(utils::SingletonLogger::DEBUG, "Stub DELETE query: " + query, __FILE__, __LINE__, __func__);
    return !query.empty();
}

bool MySQLDatabase::executeSelect(const std::string &query)
{
    logger_.logMeta(utils::SingletonLogger::DEBUG, "Stub SELECT query: " + query, __FILE__, __LINE__, __func__);
    return !query.empty();
}

std::string MySQLDatabase::escapeString(const std::string &input) const
{
    std::string escaped;
    escaped.reserve(input.size());
    for (char ch : input)
    {
        if (ch == '\\' || ch == '\'' || ch == '\"')
        {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }
    return escaped;
}

std::vector<std::map<std::string, std::string>> MySQLDatabase::fetchRows(const std::string &query) const
{
    logger_.logMeta(utils::SingletonLogger::DEBUG, "Stub fetch rows for query: " + query, __FILE__, __LINE__, __func__);
    return {};
}

} // namespace database

