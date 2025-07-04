#pragma once

#include <httplib.h>
#include <string>

#include <utils/index.h>

using namespace utils;

namespace UberBackend
{
    class SharedHttpClient
    {
    public:
        SharedHttpClient(const std::string &host, int port);
        virtual ~SharedHttpClient();

        virtual std::string get(const std::string &path) = 0;
        virtual std::string post(const std::string &path, const std::string &body, const std::string &content_type = "text/plain") = 0;

    protected:
        std::string httpClientName_;
        SingletonLogger &logger_;
        httplib::Client client_;
    };
}
