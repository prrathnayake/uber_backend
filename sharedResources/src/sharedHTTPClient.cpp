#include "../include/sharedDatabase.h"
#include "../include/sharedHTTPClient.h"

using namespace database;
using namespace UberBackend;
using namespace utils;

SharedHttpClient::SharedHttpClient(const std::string &host, int port)
    : client_(host, port), logger_(SingletonLogger::instance()) {}

SharedHttpClient::~SharedHttpClient() {}

std::string SharedHttpClient::get(const std::string &path)
{
    logger_.logMeta(SingletonLogger::INFO, "HTTP client GET: " + path, __FILE__, __LINE__, __func__);
    auto res = client_.Get(path.c_str());

    if (res && res->status == 200)
    {
        return res->body;
    }
    logger_.logMeta(SingletonLogger::ERROR, "Failed HTTP client GET: " + path, __FILE__, __LINE__, __func__);

    return "GET request failed.";
}

std::string SharedHttpClient::post(const std::string &path, const std::string &body, const std::string &content_type)
{
    logger_.logMeta(SingletonLogger::INFO, "HTTP client POST: " + path, __FILE__, __LINE__, __func__);

    auto res = client_.Post(path.c_str(), body, content_type.c_str());
    if (res && res->status == 200)
    {
        return res->body;
    }
    logger_.logMeta(SingletonLogger::ERROR, "Failed HTTP client POST: " + path, __FILE__, __LINE__, __func__);
    return "POST request failed.";
}
