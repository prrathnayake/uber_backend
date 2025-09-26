#include "../../../include/services/redisHandler/redisHandler.h"

namespace UberBackend
{

RedisHandler::RedisHandler()
    : logger_(utils::SingletonLogger::instance())
{
    logger_.logMeta(utils::SingletonLogger::INFO,
                    "Initialised in-memory Redis handler",
                    __FILE__,
                    __LINE__,
                    __func__);
}

RedisHandler::~RedisHandler() = default;

void RedisHandler::setValue(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    keyValueStore_[key] = value;
}

std::optional<std::string> RedisHandler::getValue(const std::string &key)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (auto it = keyValueStore_.find(key); it != keyValueStore_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

void RedisHandler::addDriverToSet(const std::string &h3Index, const std::string &driverId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    setStore_["available_drivers:" + h3Index].insert(driverId);
}

std::vector<std::string> RedisHandler::getDriversFromSet(const std::string &h3Index)
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto key = "available_drivers:" + h3Index;
    auto it = setStore_.find(key);
    if (it == setStore_.end())
    {
        return {};
    }

    return std::vector<std::string>(it->second.begin(), it->second.end());
}

} // namespace UberBackend
