#include "../../../../include/services/httpHandler/clients/httpClient.h"

UberBackend::HttpClient::HttpClient(const std::string& host, int port)
    : client(host, port) {}

std::string UberBackend::HttpClient::get(const std::string& path) {
    auto res = client.Get(path.c_str());
    if (res && res->status == 200) {
        return res->body;
    }
    return "GET request failed.";
}

std::string UberBackend::HttpClient::post(const std::string& path, const std::string& body, const std::string& content_type) {
    auto res = client.Post(path.c_str(), body, content_type.c_str());
    if (res && res->status == 200) {
        return res->body;
    }
    return "POST request failed.";
}
