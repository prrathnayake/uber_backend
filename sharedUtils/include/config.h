#pragma once
#include <iostream>

namespace UberBackend
{
    class UberUtils
    {
    public:
        struct CONFIG
        {
            static constexpr const char *USER_MANAGER_HOST = "localhost";
            static constexpr const char *USER_MANAGER_USERNAME = "pasan";
            static constexpr const char *USER_MANAGER_PASSWORD = "pasan";
            static constexpr const char *USER_MANAGER_DATABASE_NAME = "userManagerDatabase";
            static constexpr unsigned int USER_MANAGER_DATABASE_PORT = 3036;

            static constexpr unsigned int USER_MANAGER_HTTP_USER_HANDLER_PORT = 8081;

            static constexpr const char *LOCATION_MANAGER_HOST = "localhost";
            static constexpr const char *LOCATION_MANAGER_USERNAME = "pasan";
            static constexpr const char *LOCATION_MANAGER_PASSWORD = "pasan";
            static constexpr const char *LOCATION_MANAGER_DATABASE_NAME = "locationManagerDatabase";
            static constexpr unsigned int LOCATION_MANAGER_DATABASE_PORT = 3037;

            static constexpr unsigned int LOCATION_MANAGER_HTTP_LOCATION_HANDLER_PORT = 8082;

            static constexpr const char *RIDE_MANAGER_HOST = "localhost";
            static constexpr const char *RIDE_MANAGER_USERNAME = "pasan";
            static constexpr const char *RIDE_MANAGER_PASSWORD = "pasan";
            static constexpr const char *RIDE_MANAGER_DATABASE_NAME = "rideManagerDatabase";
            static constexpr unsigned int RIDE_MANAGER_DATABASE_PORT = 3038;

            static constexpr unsigned int RIDE_MANAGER_HTTP_RIDE_HANDLER_PORT = 8083;

            static constexpr const char *REDIS_Host = "pasan";
            static constexpr unsigned int REDIS_PORT = 6379;

            static constexpr const char *KAFKA_HOST = "localhost";
            static constexpr unsigned int KAFKA_PORT = 9092;

            static constexpr const char *JWT_SECRET = "localguwgfowgi8fgwkurvrtgwnlgeghrihtu98ynvuhfnauxehnchgecturvtigfiwgiikvbcjkbsvgwegfwfwefhofewefswefwft";
        };
    };
};