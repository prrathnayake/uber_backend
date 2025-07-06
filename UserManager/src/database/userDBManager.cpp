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

void UserDBManager::addUserToDB(std::shared_ptr<User> user)
{
    logger_.logMeta(SingletonLogger::INFO, "Inside : addUserToDB(User)", __FILE__, __LINE__, __func__);

    const std::string &role = user->getRole();

    if (role != "driver" && role != "rider")
    {
        logger_.logMeta(SingletonLogger::ERROR, "Invalid role: " + role, __FILE__, __LINE__, __func__);
        return;
    }

    if (role == "driver")
    {
        logger_.logMeta(SingletonLogger::DEBUG, "Preparing to add driver...", __FILE__, __LINE__, __func__);
    }
    else
    {
        logger_.logMeta(SingletonLogger::DEBUG, "Preparing to add rider...", __FILE__, __LINE__, __func__);
    }

    std::string query = "INSERT INTO users (first_name, middle_name, last_name, phone_number, email, username, password_hash, "
                        "country_code, role, preferred_language, currency, country) VALUES ('" +
                        database_->escapeString(user->getFirstName()) + "', '" +
                        database_->escapeString(user->getMiddleName()) + "', '" +
                        database_->escapeString(user->getLastName()) + "', '" +
                        database_->escapeString(user->getMobileNumber()) + "', '" +
                        database_->escapeString(user->getEmail()) + "', '" +
                        database_->escapeString(user->getUsername()) + "', '" +
                        database_->escapeString(user->getPasswordHash()) + "', '" +
                        database_->escapeString(user->getCountryCode()) + "', '" +
                        database_->escapeString(user->getRole()) + "', '" +
                        database_->escapeString(user->getPreferredLanguage()) + "', '" +
                        database_->escapeString(user->getCurrency()) + "', '" +
                        database_->escapeString(user->getCountry()) + "');";

    if (database_->executeInsert(query))
    {
        logger_.logMeta(SingletonLogger::INFO, "User successfully added to database.", __FILE__, __LINE__, __func__);
        // You can trigger Kafka producer here if needed
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Adding new user to database failed.", __FILE__, __LINE__, __func__);
    }
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
