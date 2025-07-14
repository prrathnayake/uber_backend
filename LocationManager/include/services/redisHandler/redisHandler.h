#pragma once

#include <memory>
#include <future>
#include <sw/redis++/redis++.h>

#include <utils/index.h>

using namespace sw::redis;
using namespace utils;

namespace UberBackend {

    class RedisHandler {
    public:
        RedisHandler();
        ~RedisHandler();

        void setValue(const std::string& key, const std::string& value);
        std::optional<std::string> getValue(const std::string& key);
        void addDriverToSet(const std::string& h3Index, const std::string& driverId);
        std::vector<std::string> getDriversFromSet(const std::string& h3Index);

    private:
        SingletonLogger &logger_;
        std::shared_ptr<Redis> redis_;
    };

}
