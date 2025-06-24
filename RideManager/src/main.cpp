#include <iostream>
#include <memory>

#include <database/index.h>

#include "database/database.h"
#include <utils/index.h>
using namespace utils;

int main()
{
    auto fileLogger = std::make_unique<FileLogger>("log/RideManagerLog.txt"); // ✅ properly initialized
    fileLogger->logMeta(FileLogger::ERROR, "Create database instance.", __FILE__, __LINE__, __func__);
};