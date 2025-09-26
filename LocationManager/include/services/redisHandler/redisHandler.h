#pragma once

#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <utils/index.h>

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
        utils::SingletonLogger &logger_;
        mutable std::mutex mutex_;
        std::unordered_map<std::string, std::string> keyValueStore_;
        std::unordered_map<std::string, std::unordered_set<std::string>> setStore_;
    };

}
