#include "../include/services/httpHandler/servers/httpUserServer.h"

using namespace UberBackend;
using namespace utils;

HttpUserServer::HttpUserServer(const std::string &name,
                               const std::string &host,
                               int port,
                               std::shared_ptr<SharedDatabase> db)
    : SharedHttpServer(name, host, port, db), jwt_("your_super_secret_key")
{
    userDBManager_ = std::make_shared<UserDBManager>(db);
}

void HttpUserServer::createServerMethods()
{
    server_->Post("/login", [this](const httplib::Request &req, httplib::Response &res)
                  {
        std::string body = req.body;
        auto jsonData = Json::parse(body);

        if (jsonData.contains("username") && jsonData.contains("password")) {
            std::string username = jsonData["username"];
            std::string passwordInput = jsonData["password"];

            logger_.logMeta(SingletonLogger::INFO, "Received login request", __FILE__, __LINE__, __func__);

            auto userData = userDBManager_->getUserByUsername(username); 

            logger_.logMeta(SingletonLogger::DEBUG, "User Data: " + userData.dump(), __FILE__, __LINE__, __func__);

            if (!userData.empty()) {
                std::string storedHash = userData["password_hash"];
                std::string hashedInput = algorithms::hashComputation(algorithms::toBinary(passwordInput));

                if (storedHash == hashedInput) {
                    std::string userId = userData["id"];
                    std::string token = jwt_.generateToken(userId);

                    logger_.logMeta(SingletonLogger::INFO, "Login successful, token generated", __FILE__, __LINE__, __func__);

                    nlohmann::json responseJson;
                    responseJson["token"] = token;
                    logger_.logMeta(SingletonLogger::DEBUG, "JWT TOKEN: " + token, __FILE__, __LINE__, __func__);

                    res.set_content(responseJson.dump(), "application/json");
                    return;
                } else {
                    logger_.logMeta(SingletonLogger::ERROR, "Password mismatch", __FILE__, __LINE__, __func__);
                }
            } else {
                logger_.logMeta(SingletonLogger::ERROR, "User not found", __FILE__, __LINE__, __func__);
            }
        } else {
            logger_.logMeta(SingletonLogger::ERROR, "Invalid login data", __FILE__, __LINE__, __func__);
        }

        res.status = 401;
        res.set_content(R"({"error": "Invalid username or password"})", "application/json"); });

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

            std::string firstName = jsonData["firstName"];
            std::string middleName = jsonData["middleName"];
            std::string lastName = jsonData["lastName"];
            std::string mobileNumber = jsonData["mobileNumber"];
            std::string address = jsonData["address"];
            std::string email = jsonData["email"];
            std::string username = jsonData["username"];
            std::string passwordHash = algorithms::hashComputation(algorithms::toBinary(jsonData["password"]));
            std::string role = jsonData["role"];

            logger_.logMeta(SingletonLogger::INFO, "Run : userDBManager_->addUserToDB()", __FILE__, __LINE__, __func__);
            userDBManager_->addUserToDB(firstName, middleName, lastName, mobileNumber, address, email, username, passwordHash, role);

            res.set_content("Signup successful", "text/plain");
        }
        else
        {
            logger_.logMeta(SingletonLogger::ERROR, "Signup data missing fields", __FILE__, __LINE__, __func__);
            res.set_content("Signup unsuccessful - missing fields", "text/plain");
        } });
}
