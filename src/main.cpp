#include <iostream>
#include <memory>

#include <database/index.h>

#include "database/database.h"
#include <utils/index.h>

int main()
{
    auto fileLogger = std::make_unique<utils::FileLogger>("log/log.txt"); // ✅ properly initialized
    fileLogger->logMeta(utils::FileLogger::ERROR, "Create database instance.", __FILE__, __LINE__, __func__);

    auto database = std::make_unique<uber_backend::uber_database>(); // pass logger to database
    database->initalizeDatabase();
    return 0;
};