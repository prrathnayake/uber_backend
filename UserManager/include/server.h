#pragma once

#include <memory>
#include <utils/index.h>
#include <database/database.h>

namespace uber_backend {

class Server {
public:
    Server();
    void initiateDatabase();

private:
    std::unique_ptr<uber_backend::uber_database> database_;
    std::unique_ptr<utils::ThreadPool> thread_pool_;
    std::unique_ptr<utils::FileLogger> fileLogger_;
};

}
