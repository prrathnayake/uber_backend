#include <iostream>

#include <utils/index.h>
#include <algorithms/sha256/index.h>

#include "../include/services/httpHandler/servers/httpUserServer.h"

using namespace uber_backend;
using namespace utils;

HttpUserServer::HttpUserServer(const std::string &host, int port, std::shared_ptr<uber_database> db)
    : HttpServer(host, port, db)
{ 
    userDBManager_ = std::make_shared<UserDBManager>(db);
}

HttpUserServer::~HttpUserServer() {}

void HttpUserServer::createServerMethods()
{
    // Get the internal server object and set up routes
    server_->Post("/login", [this](const httplib::Request &req, httplib::Response &res)
                  {
    std::string body = req.body;
    auto jsonData = Json::parse(body);

    if (jsonData.contains("username") && jsonData.contains("password"))
    {
        logger_.logMeta(SingletonLogger::INFO, "Received data successfully", __FILE__, __LINE__, __func__);
        res.set_content("Login successful", "text/plain");
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Data receiving unsuccessful", __FILE__, __LINE__, __func__);
        res.set_content("Login unsuccessful", "text/plain");
    } });

    server_->Post("/signup", [this](const httplib::Request &req, httplib::Response &res)
                  { 
        std::string body = req.body;
        auto jsonData = Json::parse(body);
        
        if (jsonData.contains("firstName") &&
            jsonData.contains("middleName") &&
            jsonData.contains("lastName") &&
            jsonData.contains("mobileNumber") &&
            jsonData.contains("address") &&
            jsonData.contains("email") &&
            jsonData.contains("username") &&
            jsonData.contains("password") &&
            jsonData.contains("role"))
                {
                    logger_.logMeta(SingletonLogger::INFO, "Received signup data successfully", __FILE__, __LINE__, __func__);

                    // Extract values
                    std::string firstName = jsonData["firstName"];
                    std::string middleName = jsonData["middleName"];
                    std::string lastName = jsonData["lastName"];
                    std::string mobileNumber = jsonData["mobileNumber"];
                    std::string address = jsonData["address"];
                    std::string email = jsonData["email"];
                    std::string username = jsonData["username"];
                    std::string passwordHash = algorithms::hashComputation(algorithms::toBinary(jsonData["password"]));
                    std::string role = jsonData["role"];

                    userDBManager_->addUserToDB(firstName, middleName, lastName, mobileNumber, address, email, username, passwordHash, role);

                    res.set_content("Signup successful", "text/plain");
                }
                else
                {
                    logger_.logMeta(SingletonLogger::ERROR, "Signup data missing fields", __FILE__, __LINE__, __func__);
                    res.set_content("Signup unsuccessful - missing fields", "text/plain");
                } });
}
