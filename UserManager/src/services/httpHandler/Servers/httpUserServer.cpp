#include "../include/services/httpHandler/servers/httpUserServer.h"

using namespace UberBackend;
using namespace utils;

HttpUserServer::HttpUserServer(const std::string &name,
                               const std::string &host,
                               int port,
                               std::shared_ptr<SharedDatabase> db)
    : SharedHttpServer(name, host, port, db), jwt_("your_super_secret_key")
{
    routHandler_ = std::make_shared<RouteHandler>(db);
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

            auto userData = routHandler_->handleUserLogin(username);

            logger_.logMeta(SingletonLogger::DEBUG, "User Data: " + userData.dump(), __FILE__, __LINE__, __func__);

            if (!userData.empty()) {
                std::string storedHash = userData["password_hash"];
                logger_.logMeta(SingletonLogger::DEBUG, "password : " + passwordInput , __FILE__, __LINE__, __func__);
                std::string hashedInput = algorithms::hashComputation(algorithms::toBinary(passwordInput));
                logger_.logMeta(SingletonLogger::DEBUG, "storedHash: " + storedHash , __FILE__, __LINE__, __func__);
                logger_.logMeta(SingletonLogger::DEBUG, "hashedInput: " + hashedInput, __FILE__, __LINE__, __func__);

                if (storedHash == hashedInput) {
                    std::string userId = userData["user_id"];
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

        auto user = std::make_shared<User>(
            firstName,
            middleName,
            lastName,
            "",                 // countryCode - not yet provided
            mobileNumber,
            address,
            email,
            username,
            passwordHash,
            role,
            "",                 // preferredLanguage
            "",                 // currency
            ""                  // country
        );

        logger_.logMeta(SingletonLogger::INFO, "Calling handleNewUser()", __FILE__, __LINE__, __func__);

        routHandler_->handleNewUser(user);

        res.set_content("Signup successful", "text/plain");
    }
    else
    {
        logger_.logMeta(SingletonLogger::ERROR, "Signup data missing fields", __FILE__, __LINE__, __func__);
        res.set_content("Signup unsuccessful - missing fields", "text/plain");
    } });

    // GET /user/:id
    server_->Get(R"(/user/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                 {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "GET /user/" + userId, __FILE__, __LINE__, __func__);

        auto userJson = routHandler_->handleGetUserById(userId);
        if (!userJson.empty()) {
            res.set_content(userJson.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error": "User not found"})", "application/json");
        } });

    // PUT /user/:id
    server_->Put(R"(/user/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                 {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "PUT /user/" + userId, __FILE__, __LINE__, __func__);

        auto jsonData = Json::parse(req.body);
        bool success = routHandler_->handleUpdateUser(userId, jsonData);
        if (success) {
            res.set_content(R"({"message": "User updated successfully"})", "application/json");
        } else {
            res.status = 400;
            res.set_content(R"({"error": "Failed to update user"})", "application/json");
        } });

    // DELETE /user/:id
    server_->Delete(R"(/user/(\d+))", [this](const httplib::Request &req, httplib::Response &res)
                    {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "DELETE /user/" + userId, __FILE__, __LINE__, __func__);

        bool deleted = routHandler_->handleDeleteUser(userId);
        if (deleted) {
            res.set_content(R"({"message": "User deleted successfully"})", "application/json");
        } else {
            res.status = 404;
            res.set_content(R"({"error": "User not found"})", "application/json");
        } });

    // GET /users
    server_->Get("/users", [this](const httplib::Request &req, httplib::Response &res)
                 {
        logger_.logMeta(SingletonLogger::INFO, "GET /users", __FILE__, __LINE__, __func__);

        auto users = routHandler_->handleGetAllUsers();
        res.set_content(users.dump(), "application/json"); });

    // PATCH /user/:id/password
    server_->Patch(R"(/user/(\d+)/password)", [this](const httplib::Request &req, httplib::Response &res)
                   {
    std::string userId = req.matches[1];
    auto jsonData = Json::parse(req.body);

    if (jsonData.contains("oldPassword") && jsonData.contains("newPassword")) {
        std::string oldPassword = jsonData["oldPassword"];
        std::string newPassword = jsonData["newPassword"];

        bool updated = routHandler_->handlePasswordUpdate(userId, oldPassword, newPassword);
        if (updated) {
            res.set_content(R"({"message": "Password updated"})", "application/json");
        } else {
            res.status = 400;
            res.set_content(R"({"error": "Incorrect old password"})", "application/json");
        }
    } else {
        res.status = 400;
        res.set_content(R"({"error": "Missing password fields"})", "application/json");
    } });

    // PATCH /user/:id/profile
    server_->Patch(R"(/user/(\d+)/profile)", [this](const httplib::Request &req, httplib::Response &res)
                   {
    std::string userId = req.matches[1];
    auto jsonData = Json::parse(req.body);

    bool updated = routHandler_->handlePartialProfileUpdate(userId, jsonData);
    if (updated) {
        res.set_content(R"({"message": "Profile updated"})", "application/json");
    } else {
        res.status = 400;
        res.set_content(R"({"error": "Failed to update profile"})", "application/json");
    } });

    // GET /user/search?username=abc
    server_->Get("/user/search", [this](const httplib::Request &req, httplib::Response &res)
                 {
    if (req.has_param("username")) {
        std::string username = req.get_param_value("username");
        auto result = routHandler_->searchUsersByUsername(username);
        res.set_content(result.dump(), "application/json");
    } else {
        res.status = 400;
        res.set_content(R"({"error": "Missing search parameter"})", "application/json");
    } });

    // GET /user/me (JWT required)
    server_->Get("/user/me", [this](const httplib::Request &req, httplib::Response &res)
                 {
    if (req.has_header("Authorization")) {
        std::string token = req.get_header_value("Authorization");
        std::string userId;

        if (jwt_.verifyToken(token, userId)) {
            auto userJson = routHandler_->handleGetUserById(userId);
            res.set_content(userJson.dump(), "application/json");
        } else {
            res.status = 401;
            res.set_content(R"({"error": "Invalid token"})", "application/json");
        }
    } else {
        res.status = 400;
        res.set_content(R"({"error": "Missing Authorization header"})", "application/json");
    } });

    // POST /user/logout
    server_->Post("/user/logout", [this](const httplib::Request &req, httplib::Response &res)
                  {
    // If session-based, invalidate session; if JWT-based, just return success
    res.set_content(R"({"message": "Logged out"})", "application/json"); });
}
