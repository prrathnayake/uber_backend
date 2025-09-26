#pragma once

#include <ctime>
#include <optional>
#include <string>
#include <tuple>

#include <utils/index.h>

namespace UberBackend {
    class JWTUtils {
    public:
        JWTUtils(const std::string &secret);

        std::string generateToken(const std::string &userId, int expirySeconds = 3600);
        bool verifyToken(const std::string &token, std::string &userId);
        std::optional<std::string> extractSubject(const std::string &token);
        std::string refreshToken(const std::string &token, int expirySeconds = 3600);

    private:
        std::string buildSignature(const std::string &userId, const std::string &salt, std::time_t expiry) const;
        std::optional<std::tuple<std::string, std::string, std::time_t, std::string>>
        parseToken(const std::string &token) const;
        std::string generateSalt() const;

        std::string secretKey_;
        utils::SingletonLogger &logger_;
    };
}
