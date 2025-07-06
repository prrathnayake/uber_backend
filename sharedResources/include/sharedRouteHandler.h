#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <utils/index.h>
#include "sharedDatabase.h"

using namespace utils;

namespace UberBackend
{
    class SharedRouteHandler
    {
    public:
        SharedRouteHandler(std::shared_ptr<SharedDatabase> db);
        ~SharedRouteHandler();

    protected:
        SingletonLogger &logger_;
        std::shared_ptr<SharedDatabase> database_;
    };

}
