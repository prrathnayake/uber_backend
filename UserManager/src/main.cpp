#include <iostream>

#include "server.h"

using namespace utils;
using namespace uber_backend;

int main()
{
    // Get singleton logger instance once (path sets the log file path)
    auto& logger = SingletonLogger::instance("log/UserServerLog.txt");
    LOG(logger, SingletonLogger::INFO, "Main started");

    // Create Server (will use the same logger instance internally)
    std::unique_ptr<Server> server = std::make_unique<Server>();
    server->initiateDatabase();

    LOG(logger, SingletonLogger::INFO, "Main ended");
    return 0;
}
