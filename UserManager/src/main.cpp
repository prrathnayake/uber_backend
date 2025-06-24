#include <iostream>
#include <string>
#include <memory>

#include "../include/server.h"

using namespace uber_backend;

int main()
{
    std::unique_ptr<Server> server = std::make_unique<Server>();
    server->initiateDatabase();
    return 0;
};