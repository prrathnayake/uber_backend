#pragma once

#include <optional>
#include <string>

#include <utils/index.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>

namespace UberBackend {
    class JWTUtils {
    public:
        JWTUtils(const std::string &secret);

        std::string generateToken(const std::string &userId, int expirySeconds = 3600);
        bool verifyToken(const std::string &token, std::string &userId);
        std::optional<std::string> extractSubject(const std::string &token);
        std::string refreshToken(const std::string &token, int expirySeconds = 3600);

    private:
        std::string secretKey_;
        utils::SingletonLogger &logger_;
    };
}
