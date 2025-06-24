#include <iostream>
#include <string>
#include <memory>

#include <database/index.h>

#include "database/database.h"
#include <utils/index.h>

using namespace utils;

int main()
{
    ThreadPool pool(64);

    auto result = pool.enqueue([](int x)
                               { return x * x; }, 8);

    auto fileLogger = std::make_unique<FileLogger>("log/UserManagerLog.txt"); // ✅ properly initialized
    fileLogger->logMeta(FileLogger::ERROR, "Create database instance.", __FILE__, __LINE__, __func__);

    auto database = std::make_unique<uber_backend::uber_database>(); // pass logger to database
    database->initalizeDatabase();
    return 0;
};