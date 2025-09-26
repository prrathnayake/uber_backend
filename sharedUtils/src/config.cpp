#include "../include/config.h"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

using namespace UberBackend;

namespace
{
    constexpr const char *DEFAULT_ENV_FILE = ".env";
}

ConfigManager &ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
    : loaded_(false), envPath_(DEFAULT_ENV_FILE), logger_(utils::SingletonLogger::instance())
{
}

void ConfigManager::loadFromFile(const std::string &envPath)
{
    std::unique_lock lock(mutex_);
    if (!envPath.empty())
    {
        envPath_ = envPath;
    }
    loaded_ = false;
    values_.clear();
}

void ConfigManager::reload()
{
    std::unique_lock lock(mutex_);
    loaded_ = false;
}

std::string ConfigManager::getString(const std::string &key, const std::string &defaultValue) const
{
    ensureLoaded();

    if (const char *envValue = std::getenv(key.c_str()); envValue != nullptr)
    {
        return std::string(envValue);
    }

    std::shared_lock lock(mutex_);
    auto it = values_.find(key);
    if (it != values_.end())
    {
        return it->second;
    }

    return defaultValue;
}

int ConfigManager::getInt(const std::string &key, int defaultValue) const
{
    const std::string value = getString(key, std::to_string(defaultValue));
    try
    {
        return std::stoi(value);
    }
    catch (const std::exception &e)
    {
        logger_.logMeta(utils::SingletonLogger::WARNING,
                        "ConfigManager::getInt fallback for key " + key + ": " + e.what(),
                        __FILE__, __LINE__, __func__);
        return defaultValue;
    }
}

unsigned int ConfigManager::getUnsigned(const std::string &key, unsigned int defaultValue) const
{
    const int parsed = getInt(key, static_cast<int>(defaultValue));
    return parsed < 0 ? defaultValue : static_cast<unsigned int>(parsed);
}

bool ConfigManager::getBool(const std::string &key, bool defaultValue) const
{
    const std::string value = getString(key, defaultValue ? "true" : "false");

    if (value == "1" || value == "true" || value == "TRUE" || value == "True")
    {
        return true;
    }
    if (value == "0" || value == "false" || value == "FALSE" || value == "False")
    {
        return false;
    }

    return defaultValue;
}

double ConfigManager::getDouble(const std::string &key, double defaultValue) const
{
    const std::string value = getString(key, std::to_string(defaultValue));
    try
    {
        return std::stod(value);
    }
    catch (const std::exception &e)
    {
        logger_.logMeta(utils::SingletonLogger::WARNING,
                        "ConfigManager::getDouble fallback for key " + key + ": " + e.what(),
                        __FILE__, __LINE__, __func__);
        return defaultValue;
    }
}

void ConfigManager::ensureLoaded() const
{
    if (loaded_)
    {
        return;
    }

    std::unique_lock lock(mutex_);
    if (!loaded_)
    {
        const_cast<ConfigManager *>(this)->loadUnlocked();
        loaded_ = true;
    }
}

void ConfigManager::loadUnlocked()
{
    values_.clear();

    if (envPath_.empty())
    {
        logger_.logMeta(utils::SingletonLogger::WARNING,
                        "ConfigManager invoked without an env path; skipping file load.",
                        __FILE__, __LINE__, __func__);
        return;
    }

    std::ifstream file(envPath_);
    if (!file.is_open())
    {
        logger_.logMeta(utils::SingletonLogger::WARNING,
                        "Unable to open env file at " + envPath_.string(),
                        __FILE__, __LINE__, __func__);
        return;
    }

    logger_.logMeta(utils::SingletonLogger::INFO,
                    "Loading configuration from " + envPath_.string(),
                    __FILE__, __LINE__, __func__);

    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        const auto separator = line.find('=');
        if (separator == std::string::npos)
        {
            continue;
        }

        std::string key = trim(line.substr(0, separator));
        std::string value = trim(line.substr(separator + 1));

        if (!key.empty())
        {
            if (!value.empty() && value.front() == '"' && value.back() == '"' && value.size() > 1)
            {
                value = value.substr(1, value.size() - 2);
            }
            values_[key] = value;
        }
    }
}

std::string ConfigManager::trim(const std::string &value)
{
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

