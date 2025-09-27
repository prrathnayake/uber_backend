#include <algorithm>
#include <optional>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../../../include/services/httpHandler/servers/httpUserServer.h"
#include "../../../../../sharedUtils/include/config.h"

using namespace UberBackend;
using namespace utils;

HttpUserServer::HttpUserServer(const std::string &name,
                               const std::string &host,
                               int port,
                               std::shared_ptr<SharedDatabase> db)
    : SharedHttpServer(name, host, port, db), jwt_(UberUtils::CONFIG::getJwtSecret())
{
    routHandler_ = std::make_shared<RouteHandler>(db);
}

void HttpUserServer::createServerMethods()
{
    using nlohmann::json;

    auto respondJson = [this](httplib::Response &res, const json &payload, int status = 200)
    {
        res.status = status;
        res.set_content(payload.dump(), "application/json");
    };

    auto parseBody = [this, respondJson](const httplib::Request &req, httplib::Response &res) -> std::optional<json>
    {
        if (req.body.empty())
        {
            logger_.logMeta(SingletonLogger::ERROR, "Empty request body received", __FILE__, __LINE__, __func__);
            respondJson(res, json{{"error", "Empty request body"}}, 400);
            return std::nullopt;
        }

        json jsonData = json::parse(req.body, nullptr, false);
        if (jsonData.is_discarded())
        {
            logger_.logMeta(SingletonLogger::ERROR, "Failed to parse JSON body", __FILE__, __LINE__, __func__);
            respondJson(res, json{{"error", "Invalid JSON payload"}}, 400);
            return std::nullopt;
        }

        return jsonData;
    };

    auto sanitizeToken = [](std::string token)
    {
        const std::string bearerPrefix = "Bearer ";
        if (token.rfind(bearerPrefix, 0) == 0)
        {
            token.erase(0, bearerPrefix.size());
        }
        return token;
    };

    auto parsePaginationParam = [](const httplib::Request &req, const char *param, int defaultValue) -> int
    {
        if (!req.has_param(param))
        {
            return defaultValue;
        }

        try
        {
            return std::stoi(req.get_param_value(param));
        }
        catch (...)
        {
            return defaultValue;
        }
    };

    server_->Get("/", [respondJson](const httplib::Request &, httplib::Response &res)
    {
        respondJson(res,
                    json{{"status", "ok"},
                         {"service", "UserManager"},
                         {"message", "User Manager HTTP interface is running"}});
    });

    server_->Post("/login", [this, respondJson, parseBody](const httplib::Request &req, httplib::Response &res)
    {
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }
        const auto &jsonData = *jsonDataOpt;

        if (!jsonData.contains("username") || !jsonData.contains("password"))
        {
            respondJson(res, json{{"error", "Missing username or password"}}, 400);
            return;
        }

        std::string username = jsonData.at("username").get<std::string>();
        std::string passwordInput = jsonData.at("password").get<std::string>();

        logger_.logMeta(SingletonLogger::INFO, "Received login request", __FILE__, __LINE__, __func__);

        auto userData = routHandler_->handleUserLogin(username);
        logger_.logMeta(SingletonLogger::DEBUG, "User Data: " + userData.dump(), __FILE__, __LINE__, __func__);

        if (!userData.empty())
        {
            std::string storedHash = userData.value("password_hash", std::string{});
            std::string hashedInput = algorithms::hashComputation(algorithms::toBinary(passwordInput));

            if (storedHash == hashedInput)
            {
                std::string userId = userData.value("user_id", std::string{});
                std::string token = jwt_.generateToken(userId);

                logger_.logMeta(SingletonLogger::INFO, "Login successful, token generated", __FILE__, __LINE__, __func__);

                respondJson(res, json{{"token", token}, {"user_id", userId}});
                return;
            }
        }

        logger_.logMeta(SingletonLogger::ERROR, "Invalid credentials supplied", __FILE__, __LINE__, __func__);
        respondJson(res, json{{"error", "Invalid username or password"}}, 401);
    });

    server_->Post("/signup", [this, respondJson, parseBody](const httplib::Request &req, httplib::Response &res)
    {
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }
        const auto &jsonData = *jsonDataOpt;

        const std::vector<std::string> requiredFields = {
            "firstName", "middleName", "lastName", "mobileNumber", "address", "email", "username", "password", "role"
        };

        std::vector<std::string> missingFields;
        for (const auto &field : requiredFields)
        {
            if (!jsonData.contains(field) || jsonData.at(field).is_null())
            {
                missingFields.push_back(field);
                continue;
            }

            const auto &value = jsonData.at(field);
            if (!value.is_string() || value.get<std::string>().empty())
            {
                missingFields.push_back(field);
            }
        }

        if (!missingFields.empty())
        {
            respondJson(res, json{{"error", "Missing required fields"}, {"fields", missingFields}}, 400);
            return;
        }

        std::string email = jsonData.at("email").get<std::string>();
        std::string username = jsonData.at("username").get<std::string>();

        auto availability = routHandler_->checkUserAvailability(username, email);
        if (availability.value("usernameTaken", false) || availability.value("emailTaken", false))
        {
            respondJson(res, json{{"error", "User already exists"}, {"details", availability}}, 409);
            return;
        }

        std::string passwordHash = algorithms::hashComputation(algorithms::toBinary(jsonData.at("password").get<std::string>()));

        auto user = std::make_shared<User>(
            jsonData.at("firstName").get<std::string>(),
            jsonData.at("middleName").get<std::string>(),
            jsonData.at("lastName").get<std::string>(),
            jsonData.value("countryCode", std::string{}),
            jsonData.at("mobileNumber").get<std::string>(),
            jsonData.at("address").get<std::string>(),
            email,
            username,
            passwordHash,
            jsonData.at("role").get<std::string>(),
            jsonData.value("preferredLanguage", std::string{}),
            jsonData.value("currency", std::string{}),
            jsonData.value("country", std::string{})
        );

        logger_.logMeta(SingletonLogger::INFO, "Calling handleNewUser()", __FILE__, __LINE__, __func__);

        if (routHandler_->handleNewUser(user))
        {
            respondJson(res, json{{"message", "Signup successful"}}, 201);
        }
        else
        {
            respondJson(res, json{{"error", "Signup unsuccessful"}}, 409);
        }
    });

    server_->Get(R"(/user/(\d+))", [this, respondJson](const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "GET /user/" + userId, __FILE__, __LINE__, __func__);

        auto userJson = routHandler_->handleGetUserById(userId);
        if (!userJson.empty())
        {
            respondJson(res, userJson);
        }
        else
        {
            respondJson(res, json{{"error", "User not found"}}, 404);
        }
    });

    server_->Put(R"(/user/(\d+))", [this, respondJson, parseBody](const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "PUT /user/" + userId, __FILE__, __LINE__, __func__);

        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }

        bool success = routHandler_->handleUpdateUser(userId, *jsonDataOpt);
        if (success)
        {
            respondJson(res, json{{"message", "User updated successfully"}});
        }
        else
        {
            respondJson(res, json{{"error", "Failed to update user"}}, 400);
        }
    });

    server_->Delete(R"(/user/(\d+))", [this, respondJson](const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1];
        logger_.logMeta(SingletonLogger::INFO, "DELETE /user/" + userId, __FILE__, __LINE__, __func__);

        bool deleted = routHandler_->handleDeleteUser(userId);
        if (deleted)
        {
            respondJson(res, json{{"message", "User deleted successfully"}});
        }
        else
        {
            respondJson(res, json{{"error", "User not found"}}, 404);
        }
    });

    server_->Get("/users", [this, respondJson](const httplib::Request &, httplib::Response &res)
    {
        logger_.logMeta(SingletonLogger::INFO, "GET /users", __FILE__, __LINE__, __func__);
        respondJson(res, routHandler_->handleGetAllUsers());
    });

    server_->Get("/users/paginated", [this, respondJson, parsePaginationParam](const httplib::Request &req, httplib::Response &res)
    {
        int offset = std::max(parsePaginationParam(req, "offset", 0), 0);
        int limit = std::clamp(parsePaginationParam(req, "limit", 25), 1, 100);

        json response;
        response["data"] = routHandler_->handleGetUsersPaginated(offset, limit);
        response["meta"] = {
            {"offset", offset},
            {"limit", limit}
        };

        respondJson(res, response);
    });

    server_->Get("/users/stats", [this, respondJson](const httplib::Request &, httplib::Response &res)
    {
        respondJson(res, routHandler_->getUserStats());
    });

    server_->Get("/user/exists", [this, respondJson](const httplib::Request &req, httplib::Response &res)
    {
        std::string username = req.has_param("username") ? req.get_param_value("username") : std::string{};
        std::string email = req.has_param("email") ? req.get_param_value("email") : std::string{};

        if (username.empty() && email.empty())
        {
            respondJson(res, json{{"error", "Provide username or email to check"}}, 400);
            return;
        }

        respondJson(res, routHandler_->checkUserAvailability(username, email));
    });

    server_->Patch(R"(/user/(\d+)/password)", [this, respondJson, parseBody](const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1];
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }

        const auto &jsonData = *jsonDataOpt;
        if (!jsonData.contains("oldPassword") || !jsonData.contains("newPassword"))
        {
            respondJson(res, json{{"error", "Missing password fields"}}, 400);
            return;
        }

        bool updated = routHandler_->handlePasswordUpdate(
            userId,
            jsonData.at("oldPassword").get<std::string>(),
            jsonData.at("newPassword").get<std::string>());

        if (updated)
        {
            respondJson(res, json{{"message", "Password updated"}});
        }
        else
        {
            respondJson(res, json{{"error", "Incorrect old password"}}, 400);
        }
    });

    server_->Patch(R"(/user/(\d+)/profile)", [this, respondJson, parseBody](const httplib::Request &req, httplib::Response &res)
    {
        std::string userId = req.matches[1];
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }

        bool updated = routHandler_->handlePartialProfileUpdate(userId, *jsonDataOpt);
        if (updated)
        {
            respondJson(res, json{{"message", "Profile updated"}});
        }
        else
        {
            respondJson(res, json{{"error", "Failed to update profile"}}, 400);
        }
    });

    server_->Get("/user/search", [this, respondJson](const httplib::Request &req, httplib::Response &res)
    {
        if (!req.has_param("username"))
        {
            respondJson(res, json{{"error", "Missing search parameter"}}, 400);
            return;
        }

        std::string username = req.get_param_value("username");
        respondJson(res, routHandler_->searchUsersByUsername(username));
    });

    server_->Get("/user/me", [this, respondJson, sanitizeToken](const httplib::Request &req, httplib::Response &res)
    {
        if (!req.has_header("Authorization"))
        {
            respondJson(res, json{{"error", "Missing Authorization header"}}, 400);
            return;
        }

        std::string token = sanitizeToken(req.get_header_value("Authorization"));
        std::string userId;

        if (jwt_.verifyToken(token, userId))
        {
            auto userJson = routHandler_->handleGetUserById(userId);
            respondJson(res, userJson);
        }
        else
        {
            respondJson(res, json{{"error", "Invalid token"}}, 401);
        }
    });

    server_->Post("/user/refresh-token", [this, respondJson, parseBody, sanitizeToken](const httplib::Request &req, httplib::Response &res)
    {
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }

        const auto &jsonData = *jsonDataOpt;
        if (!jsonData.contains("token"))
        {
            respondJson(res, json{{"error", "Missing token field"}}, 400);
            return;
        }

        std::string token = sanitizeToken(jsonData.at("token").get<std::string>());
        std::string newToken = jwt_.refreshToken(token);

        if (newToken.empty())
        {
            respondJson(res, json{{"error", "Invalid or expired token"}}, 401);
            return;
        }

        auto subject = jwt_.extractSubject(newToken);
        respondJson(res, json{{"token", newToken}, {"user_id", subject.value_or(std::string{})}});
    });

    server_->Post("/user/token/introspect", [this, respondJson, parseBody, sanitizeToken](const httplib::Request &req, httplib::Response &res)
    {
        auto jsonDataOpt = parseBody(req, res);
        if (!jsonDataOpt)
        {
            return;
        }

        const auto &jsonData = *jsonDataOpt;
        if (!jsonData.contains("token"))
        {
            respondJson(res, json{{"error", "Missing token field"}}, 400);
            return;
        }

        std::string token = sanitizeToken(jsonData.at("token").get<std::string>());
        std::string userId;

        if (!jwt_.verifyToken(token, userId))
        {
            respondJson(res, json{{"valid", false}}, 401);
            return;
        }

        respondJson(res, json{{"valid", true}, {"user_id", userId}});
    });

    server_->Get("/health", [this, respondJson](const httplib::Request &, httplib::Response &res)
    {
        respondJson(res, routHandler_->getHealthStatus());
    });

    server_->Post("/user/logout", [respondJson](const httplib::Request &, httplib::Response &res)
    {
        respondJson(res, json{{"message", "Logged out"}});
    });
}
