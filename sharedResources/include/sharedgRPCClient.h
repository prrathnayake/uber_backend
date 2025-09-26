#pragma once

#include <string>

#include <utils/index.h>

namespace UberBackend
{

    class LocationClient
    {
    public:
        explicit LocationClient(const std::string &endpoint = "localhost:50051");

        std::string SendLocation(const std::string &userID, double lat, double lon);

    private:
        utils::SingletonLogger &logger_;
        std::string endpoint_;
    };

}
