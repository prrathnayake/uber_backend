#include "../include/server.h"
#include "utils/index.h" // Adjust path if needed

using namespace utils;
using namespace uber_backend;

Server::Server()
{
    SingletonLogger &logger = SingletonLogger::instance();
    thread_pool_ = std::make_unique<ThreadPool>(64);

    logger.logMeta(SingletonLogger::INFO, "Server initialized", __FILE__, __LINE__, __func__);
}

void Server::initiateDatabase()
{
    SingletonLogger &logger = SingletonLogger::instance();

    auto result = thread_pool_->enqueue([](int x)
                                        { return x * x; }, 8);

    logger.logMeta(SingletonLogger::ERROR, "Creating database instance.", __FILE__, __LINE__, __func__);

    database_ = std::make_unique<uber_database>();
    database_->initalizeDatabase(); // Pass logger/config if needed
}
