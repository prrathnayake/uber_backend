#include <iostream>
#include <string>
#include <filesystem>

#include "../include/sharedRouteHandler.h"

using namespace database;
using namespace UberBackend;
using namespace utils;

SharedRouteHandler::SharedRouteHandler(std::shared_ptr<SharedDatabase> db)
    : logger_(SingletonLogger::instance()), database_(db)

{
}

SharedRouteHandler::~SharedRouteHandler()
{
}