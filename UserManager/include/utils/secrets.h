#include <iostream>

namespace uber_backend
{
    class uber_utils
    {
    public:
        struct secrets_database
        {
            static constexpr const char *HOST = "localhost";
            static constexpr const char *USERNAME = "pasan";
            static constexpr const char *PASSWORD = "pasan";
            static constexpr const char *DATABASE_NAME = "uber_backend";
            static constexpr unsigned int DATABASE_PORT = 3306;
        };
    };
};