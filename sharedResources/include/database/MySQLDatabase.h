#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <utils/index.h>

namespace database {

class MySQLDatabase {
public:
    MySQLDatabase(std::string host,
                  std::string user,
                  std::string password,
                  unsigned int port,
                  std::string databaseName);

    bool runSqlScript(const std::string &path);
    bool executeInsert(const std::string &query);
    bool executeUpdate(const std::string &query);
    bool executeDelete(const std::string &query);
    bool executeSelect(const std::string &query);
    std::string escapeString(const std::string &input) const;
    std::vector<std::map<std::string, std::string>> fetchRows(const std::string &query) const;

private:
    std::string host_;
    std::string user_;
    std::string password_;
    unsigned int port_;
    std::string databaseName_;

    mutable std::mutex mutex_;
    utils::SingletonLogger &logger_;
};

} // namespace database

