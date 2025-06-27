#include <iostream>
#include <string>

#include <utils/index.h>

#include "../../include/database/userDBManager.h"

using namespace uber_backend;
using namespace utils;

UserDBManager::UserDBManager(std::shared_ptr<uber_backend::uber_database> db)
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

    logger_.logMeta(SingletonLogger::INFO, "Quert printed successfully", __FILE__, __LINE__, __func__);
    std::cerr << query + "\n";
    database_->executeInsert(query);
}

void UserDBManager::getUserByID() {};