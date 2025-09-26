#include <jwt-cpp/jwt.h>

#include "../include/jwt.h"

using namespace UberBackend;

JWTUtils::JWTUtils(const std::string &secret)
    : secretKey_(secret), logger_(utils::SingletonLogger::instance()) {}

std::string JWTUtils::generateToken(const std::string &userId, int expirySeconds) {
    try {
        // Use explicit traits if required
        using json_traits = jwt::traits::nlohmann_json;

        auto token = jwt::create<json_traits>()
            .set_issuer("uber-backend")
            .set_type("JWT")
            .set_payload_claim("sub", jwt::claim(userId))
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{expirySeconds})
            .sign(jwt::algorithm::hs256{secretKey_});

        return token;
    } catch (const std::exception &e) {
        logger_.logMeta(utils::SingletonLogger::ERROR, "JWT generation failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
        return "";
    }
}

bool JWTUtils::verifyToken(const std::string &token, std::string &userId) {
    try {
        using json_traits = jwt::traits::nlohmann_json;

        auto decoded = jwt::decode<json_traits>(token);

        auto verifier = jwt::verify<json_traits>(jwt::default_clock{})
            .allow_algorithm(jwt::algorithm::hs256{secretKey_})
            .with_issuer("uber-backend");

        verifier.verify(decoded);

        if (decoded.has_payload_claim("sub")) {
            userId = decoded.get_payload_claim("sub").as_string();
            return true;
        }
    } catch (const std::exception &e) {
        logger_.logMeta(utils::SingletonLogger::ERROR, "JWT verification failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
    }

    return false;
}

std::optional<std::string> JWTUtils::extractSubject(const std::string &token) {
    try {
        using json_traits = jwt::traits::nlohmann_json;
        auto decoded = jwt::decode<json_traits>(token);

        if (decoded.has_payload_claim("sub")) {
            return decoded.get_payload_claim("sub").as_string();
        }
    } catch (const std::exception &e) {
        logger_.logMeta(utils::SingletonLogger::ERROR, "JWT decode failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
    }

    return std::nullopt;
}

std::string JWTUtils::refreshToken(const std::string &token, int expirySeconds) {
    std::string userId;
    if (!verifyToken(token, userId)) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Refresh denied due to invalid token", __FILE__, __LINE__, __func__);
        return "";
    }

    return generateToken(userId, expirySeconds);
}

