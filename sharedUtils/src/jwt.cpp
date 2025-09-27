#include "../include/jwt.h"

#include <algorithms/sha256/index.h>

#include <chrono>
#include <random>
#include <sstream>
#include <vector>

using namespace UberBackend;

JWTUtils::JWTUtils(const std::string &secret)
    : secretKey_(secret), logger_(utils::SingletonLogger::instance()) {}

std::string JWTUtils::generateToken(const std::string &userId, int expirySeconds) {
    try {
        const auto now = std::chrono::system_clock::now();
        const auto expiry = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + expirySeconds;
        const std::string salt = generateSalt();
        const std::string signature = buildSignature(userId, salt, expiry);

        std::ostringstream oss;
        oss << userId << '.' << salt << '.' << expiry << '.' << signature;
        return oss.str();
    } catch (const std::exception &e) {
        logger_.logMeta(utils::SingletonLogger::ERROR, "Token generation failed: " + std::string(e.what()), __FILE__, __LINE__, __func__);
        return "";
    }
}

bool JWTUtils::verifyToken(const std::string &token, std::string &userId) {
    const auto parsed = parseToken(token);
    if (!parsed.has_value()) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Token verification failed: malformed token", __FILE__, __LINE__, __func__);
        return false;
    }

    const auto &[id, salt, expiry, signature] = parsed.value();
    const std::string expectedSignature = buildSignature(id, salt, expiry);

    if (expectedSignature != signature) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Token verification failed: signature mismatch", __FILE__, __LINE__, __func__);
        return false;
    }

    const auto now = std::chrono::system_clock::now();
    const auto currentEpoch = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    if (expiry < currentEpoch) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Token verification failed: token expired", __FILE__, __LINE__, __func__);
        return false;
    }

    userId = id;
    return true;
}

std::optional<std::string> JWTUtils::extractSubject(const std::string &token) {
    const auto parsed = parseToken(token);
    if (!parsed.has_value()) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Token subject extraction failed: malformed token", __FILE__, __LINE__, __func__);
        return std::nullopt;
    }

    const auto &[id, salt, expiry, signature] = parsed.value();
    const std::string expectedSignature = buildSignature(id, salt, expiry);
    if (expectedSignature != signature) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Token subject extraction failed: signature mismatch", __FILE__, __LINE__, __func__);
        return std::nullopt;
    }

    return id;
}

std::string JWTUtils::refreshToken(const std::string &token, int expirySeconds) {
    std::string userId;
    if (!verifyToken(token, userId)) {
        logger_.logMeta(utils::SingletonLogger::WARNING, "Refresh denied due to invalid token", __FILE__, __LINE__, __func__);
        return "";
    }

    return generateToken(userId, expirySeconds);
}

std::string JWTUtils::buildSignature(const std::string &userId, const std::string &salt, std::time_t expiry) const {
    const std::string data = userId + ':' + salt + ':' + std::to_string(expiry) + ':' + secretKey_;
    const std::string binary = algorithms::toBinary(data);
    return algorithms::hashComputation(binary);
}

std::optional<std::tuple<std::string, std::string, std::time_t, std::string>>
JWTUtils::parseToken(const std::string &token) const {
    if (token.empty()) {
        return std::nullopt;
    }

    std::vector<std::string> segments;
    std::string current;
    for (char ch : token) {
        if (ch == '.') {
            segments.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    segments.push_back(current);

    if (segments.size() != 4) {
        return std::nullopt;
    }

    try {
        const std::string &userId = segments[0];
        const std::string &salt = segments[1];
        const std::time_t expiry = static_cast<std::time_t>(std::stoll(segments[2]));
        const std::string &signature = segments[3];
        if (userId.empty() || salt.empty() || signature.empty()) {
            return std::nullopt;
        }
        return std::make_tuple(userId, salt, expiry, signature);
    } catch (const std::exception &) {
        return std::nullopt;
    }
}

std::string JWTUtils::generateSalt() const {
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 61);
    constexpr std::size_t saltLength = 16;
    const char alphabet[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::string salt;
    salt.reserve(saltLength);
    for (std::size_t i = 0; i < saltLength; ++i) {
        salt.push_back(alphabet[dist(rng)]);
    }
    return salt;
}

