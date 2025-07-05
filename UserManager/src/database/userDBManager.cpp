#include <iostream>
#include <string>

#include <utils/index.h>

#include "../../include/database/userDBManager.h"

using namespace UberBackend;
using namespace utils;

UserDBManager::UserDBManager(std::shared_ptr<UberBackend::SharedDatabase> db)
    : database_(db), logger_(SingletonLogger::instance())
{
}

UserDBManager::~UserDBManager() {}

void UserDBManager::addUserToDB(const std::string &firstName,
                                const std::string &middleName,
                                const std::string &lastName,
                                const std::string &mobileNumber,
                                const std::string &address,
                                const std::string &email,
                                const std::string &username,
                                const std::string &password,
                                const std::string &role)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside : addUserToDB()", __FILE__, __LINE__, __func__);
    std::string query = "INSERT INTO users (first_name, middle_name, last_name, phone, email, username, password_hash, address, role ) VALUES ('" +
                        database_->escapeString(firstName) + "', '" +
                        database_->escapeString(middleName) + "', '" +
                        database_->escapeString(lastName) + "', '" +
                        database_->escapeString(mobileNumber) + "', '" +
                        database_->escapeString(email) + "', '" +
                        database_->escapeString(username) + "', '" +
                        database_->escapeString(password) + "', '" +
                        database_->escapeString(address) + "', '" +
                        database_->escapeString(role) + "');";

    database_->executeInsert(query);
}

nlohmann::json UserDBManager::getUserByID(int userID)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside : getUserByID()", __FILE__, __LINE__, __func__);

    std::string query = "SELECT * FROM users WHERE id = " + std::to_string(userID) + ";";
    auto rows = database_->fetchRows(query);

    if (!rows.empty())
    {
        return nlohmann::json(rows[0]);
    }

    return nlohmann::json(); // empty
}

nlohmann::json UserDBManager::getUserByUsername(const std::string &username)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside : getUserByUsername()", __FILE__, __LINE__, __func__);

    std::string query = "SELECT * FROM users WHERE username = '" + database_->escapeString(username) + "';";
    auto rows = database_->fetchRows(query);

    if (!rows.empty())
    {
        return nlohmann::json(rows[0]);
    }

    return nlohmann::json(); // empty
}
