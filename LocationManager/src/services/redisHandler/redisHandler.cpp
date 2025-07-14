#include "../../../include/services/redisHandler/redisHandler.h"
#include <sw/redis++/redis++.h>
#include <optional>

using namespace UberBackend;
using namespace sw::redis;

RedisHandler::RedisHandler() : logger_(SingletonLogger::instance())
{
    try
    {
        redis_ = std::make_shared<Redis>("tcp://127.0.0.1:6379");
        logger_.logMeta(SingletonLogger::INFO, "Connected to Redis", __FILE__, __LINE__, __func__);
    }
    catch (const sw::redis::Error &e)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Failed to connect to Redis: " + std::string(e.what()), __FILE__, __LINE__, __func__);
    }
}

RedisHandler::~RedisHandler() = default;

void RedisHandler::setValue(const std::string &key, const std::string &value)
{
    try
    {
        redis_->set(key, value);
    }
    catch (const sw::redis::Error &e)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Redis SET failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
    }
}

std::optional<std::string> RedisHandler::getValue(const std::string &key)
{
    try
    {
        return redis_->get(key);
    }
    catch (const sw::redis::Error &e)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Redis GET failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
        return std::nullopt;
    }
}

void RedisHandler::addDriverToSet(const std::string &h3Index, const std::string &driverId)
{
    try
    {
        redis_->sadd("available_drivers:" + h3Index, driverId);
    }
    catch (const sw::redis::Error &e)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Redis SADD failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
    }
}

std::vector<std::string> RedisHandler::getDriversFromSet(const std::string &h3Index)
{
    try
    {
        std::vector<std::string> result;
        redis_->smembers("available_drivers:" + h3Index, std::back_inserter(result));
        return result;
    }
    catch (const sw::redis::Error &e)
    {
        logger_.logMeta(SingletonLogger::ERROR, "Redis SMEMBERS failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
        return {};
    }
}
