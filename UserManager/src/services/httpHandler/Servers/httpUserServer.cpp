#include <iostream>
#include "../include/services/httpHandler/servers/httpUserServer.h"

using namespace uber_backend;

HttpUserServer::HttpUserServer(const std::string &host, int port)
    : HttpServer(host, port) {}

HttpUserServer::~HttpUserServer() {}

void HttpUserServer::createServerMethods()
{
    // Get the internal server object and set up routes
    server_->Post("/login", [](const httplib::Request &req, httplib::Response &res){ 
        std::string body = req.body;
        std::cout << "Login request received: " << body << std::endl;
        res.set_content("Login successful", "text/plain"); });

    server_->Post("/signup", [](const httplib::Request &req, httplib::Response &res){ 
        std::string body = req.body;
        std::cout << "Signup request received: " << body << std::endl;
        res.set_content("Signup successful", "text/plain"); 
    });
}
