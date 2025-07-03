#pragma once
#include <httplib.h>
#include <string>

namespace UberBackend
{
    class HttpClient
    {
    public:
        HttpClient(const std::string &host, int port);

        std::string get(const std::string &path);
        std::string post(const std::string &path, const std::string &body, const std::string &content_type = "text/plain");

    private:
        httplib::Client client;
    };
};