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

            static constexpr const char *KAFKA_HOST = "localhost";
            static constexpr unsigned int KAFKA_PORT = 9092;

            static constexpr const char *JWT_SECRET = "localguwgfowgi8fgwkurvrtgwnlgeghrihtu98ynvuhfnauxehnchgecturvtigfiwgiikvbcjkbsvgwegfwfwefhofewefswefwft";
        };
    };
};