#pragma once
#include <iostream>

namespace uber_backend
{
    class uber_utils
    {
    public:
        struct CONFIG
        {
            static constexpr const char *HOST = "localhost";
            static constexpr const char *USERNAME = "pasan";
            static constexpr const char *PASSWORD = "pasan";
            static constexpr const char *DATABASE_NAME = "uber_backend";
            static constexpr unsigned int DATABASE_PORT = 3306;

            static constexpr unsigned int HTTP_USER_HANDLER_PORT = 8081;
        };
    };
};