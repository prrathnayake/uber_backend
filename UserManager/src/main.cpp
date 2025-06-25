#include <iostream>

#include "server.h"

using namespace utils;
using namespace uber_backend;

int main()
{
    // Get singleton logger to use same instance everywhere
    auto &logger_ = SingletonLogger::instance("log/UserServerLog.txt");

    logger_.logMeta(SingletonLogger::INFO, "Main started", __FILE__, __LINE__, __func__);

    std::unique_ptr<Server> server = std::make_unique<Server>(); // create a server for the application
    server->initiateDatabase(); // initiate aplication database
    server->startHttpServers(); // start HTTP servers

    std::cout << "Press Enter to stop server...\n";
    std::cin.get();

    server->stopHttpServers(); // stop HTTP servers

    logger_.logMeta(SingletonLogger::INFO, "Main ended", __FILE__, __LINE__, __func__);
    return 0;
}