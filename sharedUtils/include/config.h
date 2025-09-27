#pragma once

#include <filesystem>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include <utils/index.h>

namespace UberBackend
{
    /**
     * @brief Runtime configuration loader with support for environment overrides and .env files.
     *
     * The legacy code relied purely on compile-time constants.  This manager keeps those defaults
     * while allowing deployments to override values without recompilation.
     */
    class ConfigManager
    {
    public:
        static ConfigManager &instance();

        /**
         * @brief Configure the manager to read from the provided .env file.
         *        The file is parsed lazily on first access, and cached values are invalidated.
         */
        void loadFromFile(const std::string &envPath);

        /**
         * @brief Force reloading configuration values from disk on the next read.
         */
        void reload();

        [[nodiscard]] std::string getString(const std::string &key, const std::string &defaultValue = "") const;
        [[nodiscard]] int getInt(const std::string &key, int defaultValue = 0) const;
        [[nodiscard]] unsigned int getUnsigned(const std::string &key, unsigned int defaultValue = 0) const;
        [[nodiscard]] bool getBool(const std::string &key, bool defaultValue = false) const;
        [[nodiscard]] double getDouble(const std::string &key, double defaultValue = 0.0) const;

    private:
        ConfigManager();

        void ensureLoaded() const;
        void loadUnlocked();
        static std::string trim(const std::string &value);

        mutable std::shared_mutex mutex_;
        mutable bool loaded_;
        std::unordered_map<std::string, std::string> values_;
        std::filesystem::path envPath_;
        utils::SingletonLogger &logger_;
    };

    class UberUtils
    {
    public:
        struct CONFIG
        {
            static constexpr const char *USER_MANAGER_HOST = "localhost";
            static constexpr const char *USER_MANAGER_USERNAME = "pasan";
            static constexpr const char *USER_MANAGER_PASSWORD = "pasan";
            static constexpr const char *USER_MANAGER_DATABASE_HOST = "localhost";
            static constexpr const char *USER_MANAGER_DATABASE_NAME = "userManagerDatabase";
            static constexpr unsigned int USER_MANAGER_DATABASE_PORT = 3036;

            static constexpr unsigned int USER_MANAGER_HTTP_USER_HANDLER_PORT = 8081;

            static constexpr const char *LOCATION_MANAGER_HOST = "localhost";
            static constexpr const char *LOCATION_MANAGER_USERNAME = "pasan";
            static constexpr const char *LOCATION_MANAGER_PASSWORD = "pasan";
            static constexpr const char *LOCATION_MANAGER_DATABASE_HOST = "localhost";
            static constexpr const char *LOCATION_MANAGER_DATABASE_NAME = "locationManagerDatabase";
            static constexpr unsigned int LOCATION_MANAGER_DATABASE_PORT = 3037;

            static constexpr unsigned int LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT = 8082;
            static constexpr unsigned int LOCATION_MANAGER_GRPC_PORT = 50051;

            static constexpr const char *RIDE_MANAGER_HOST = "localhost";
            static constexpr const char *RIDE_MANAGER_USERNAME = "pasan";
            static constexpr const char *RIDE_MANAGER_PASSWORD = "pasan";
            static constexpr const char *RIDE_MANAGER_DATABASE_HOST = "localhost";
            static constexpr const char *RIDE_MANAGER_DATABASE_NAME = "rideManagerDatabase";
            static constexpr unsigned int RIDE_MANAGER_DATABASE_PORT = 3038;

            static constexpr unsigned int RIDE_MANAGER_HTTP_RIDE_HANDLER_PORT = 8083;

            static constexpr const char *REDIS_HOST = "pasan";
            static constexpr unsigned int REDIS_PORT = 6379;

            static constexpr const char *KAFKA_HOST = "localhost";
            static constexpr unsigned int KAFKA_PORT = 9092;

            static constexpr const char *RABBITMQ_HOST = "localhost";
            static constexpr unsigned int RABBITMQ_PORT = 5672;
            static constexpr const char *RABBITMQ_USERNAME = "guest";
            static constexpr const char *RABBITMQ_PASSWORD = "guest";
            static constexpr const char *RABBITMQ_VHOST = "/";

            static constexpr const char *JWT_SECRET = "localguwgfowgi8fgwkurvrtgwnlgeghrihtu98ynvuhfnauxehnchgecturvtigfiwgiikvb"
                                                      "cjkbsvgwegfwfwefhofewefswefwft";

            static std::string getUserManagerHost()
            {
                return ConfigManager::instance().getString("USER_MANAGER_HOST", USER_MANAGER_HOST);
            }

            static std::string getUserManagerUsername()
            {
                return ConfigManager::instance().getString("USER_MANAGER_USERNAME", USER_MANAGER_USERNAME);
            }

            static std::string getUserManagerPassword()
            {
                return ConfigManager::instance().getString("USER_MANAGER_PASSWORD", USER_MANAGER_PASSWORD);
            }

            static std::string getUserManagerDatabaseHost()
            {
                return ConfigManager::instance().getString("USER_MANAGER_DATABASE_HOST", USER_MANAGER_DATABASE_HOST);
            }

            static std::string getUserManagerDatabase()
            {
                return ConfigManager::instance().getString("USER_MANAGER_DATABASE_NAME", USER_MANAGER_DATABASE_NAME);
            }

            static unsigned int getUserManagerDatabasePort()
            {
                return ConfigManager::instance().getUnsigned("USER_MANAGER_DATABASE_PORT", USER_MANAGER_DATABASE_PORT);
            }

            static unsigned int getUserManagerHttpPort()
            {
                return ConfigManager::instance().getUnsigned("USER_MANAGER_HTTP_USER_HANDLER_PORT", USER_MANAGER_HTTP_USER_HANDLER_PORT);
            }

            static std::string getLocationManagerHost()
            {
                return ConfigManager::instance().getString("LOCATION_MANAGER_HOST", LOCATION_MANAGER_HOST);
            }

            static std::string getLocationManagerUsername()
            {
                return ConfigManager::instance().getString("LOCATION_MANAGER_USERNAME", LOCATION_MANAGER_USERNAME);
            }

            static std::string getLocationManagerPassword()
            {
                return ConfigManager::instance().getString("LOCATION_MANAGER_PASSWORD", LOCATION_MANAGER_PASSWORD);
            }

            static std::string getLocationManagerDatabaseHost()
            {
                return ConfigManager::instance().getString("LOCATION_MANAGER_DATABASE_HOST", LOCATION_MANAGER_DATABASE_HOST);
            }

            static std::string getLocationManagerDatabase()
            {
                return ConfigManager::instance().getString("LOCATION_MANAGER_DATABASE_NAME", LOCATION_MANAGER_DATABASE_NAME);
            }

            static unsigned int getLocationManagerDatabasePort()
            {
                return ConfigManager::instance().getUnsigned("LOCATION_MANAGER_DATABASE_PORT", LOCATION_MANAGER_DATABASE_PORT);
            }

            static unsigned int getLocationManagerHttpPort()
            {
                return ConfigManager::instance().getUnsigned("LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT", LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT);
            }

            static std::string getKafkaHost()
            {
                return ConfigManager::instance().getString("KAFKA_HOST", KAFKA_HOST);
            }

            static unsigned int getKafkaPort()
            {
                return ConfigManager::instance().getUnsigned("KAFKA_PORT", KAFKA_PORT);
            }

            static unsigned int getLocationManagerGrpcPort()
            {
                return ConfigManager::instance().getUnsigned("LOCATION_MANAGER_GRPC_PORT", LOCATION_MANAGER_GRPC_PORT);
            }

            static std::string getRideManagerHost()
            {
                return ConfigManager::instance().getString("RIDE_MANAGER_HOST", RIDE_MANAGER_HOST);
            }

            static std::string getRideManagerDatabaseHost()
            {
                return ConfigManager::instance().getString("RIDE_MANAGER_DATABASE_HOST", RIDE_MANAGER_DATABASE_HOST);
            }

            static unsigned int getRideManagerDatabasePort()
            {
                return ConfigManager::instance().getUnsigned("RIDE_MANAGER_DATABASE_PORT", RIDE_MANAGER_DATABASE_PORT);
            }

            static std::string getRideManagerDatabase()
            {
                return ConfigManager::instance().getString("RIDE_MANAGER_DATABASE_NAME", RIDE_MANAGER_DATABASE_NAME);
            }

            static std::string getRideManagerUsername()
            {
                return ConfigManager::instance().getString("RIDE_MANAGER_USERNAME", RIDE_MANAGER_USERNAME);
            }

            static std::string getRideManagerPassword()
            {
                return ConfigManager::instance().getString("RIDE_MANAGER_PASSWORD", RIDE_MANAGER_PASSWORD);
            }

            static unsigned int getRideManagerHttpPort()
            {
                return ConfigManager::instance().getUnsigned("RIDE_MANAGER_HTTP_RIDE_HANDLER_PORT", RIDE_MANAGER_HTTP_RIDE_HANDLER_PORT);
            }

            static std::string getRedisHost()
            {
                return ConfigManager::instance().getString("REDIS_HOST", REDIS_HOST);
            }

            static unsigned int getRedisPort()
            {
                return ConfigManager::instance().getUnsigned("REDIS_PORT", REDIS_PORT);
            }

            static std::string getRabbitMQHost()
            {
                return ConfigManager::instance().getString("RABBITMQ_HOST", RABBITMQ_HOST);
            }

            static unsigned int getRabbitMQPort()
            {
                return ConfigManager::instance().getUnsigned("RABBITMQ_PORT", RABBITMQ_PORT);
            }

            static std::string getRabbitMQUsername()
            {
                return ConfigManager::instance().getString("RABBITMQ_USERNAME", RABBITMQ_USERNAME);
            }

            static std::string getRabbitMQPassword()
            {
                return ConfigManager::instance().getString("RABBITMQ_PASSWORD", RABBITMQ_PASSWORD);
            }

            static std::string getRabbitMQVHost()
            {
                return ConfigManager::instance().getString("RABBITMQ_VHOST", RABBITMQ_VHOST);
            }

            static std::string getJwtSecret()
            {
                return ConfigManager::instance().getString("JWT_SECRET", JWT_SECRET);
            }
        };
    };
}

