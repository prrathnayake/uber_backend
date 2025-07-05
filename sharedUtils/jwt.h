#pragma once

#include <chrono>
#include <iostream>
#include <jwt-cpp/jwt.h>

namespace UberBackend
{
    class JWTUtils
    {
    public:
        JWTUtils(const std::string &secret);

        std::string generateToken(const std::string &userId, int expireSeconds = 3600);
        bool verifyToken(const std::string &token, std::string &userIdOut);

    private:
        std::string secretKey;
    };
};

namespace UberBackend
{

    JWTUtils::JWTUtils(const std::string &secret) : secretKey(secret) {}

    std::string JWTUtils::generateToken(const std::string &userId, int expireSeconds)
    {
        using namespace std::chrono;

        auto now = system_clock::now();

        auto token = jwt::create()
                         .set_issuer("uber_backend")
                         .set_type("JWT")
                         .set_subject(userId)
                         .set_issued_at(now)
                         .set_expires_at(now + seconds{expireSeconds})
                         .sign(jwt::algorithm::hs256{secretKey});

        return token;
    }

    bool JWTUtils::verifyToken(const std::string &token, std::string &userIdOut)
    {
        try
        {
            auto decoded = jwt::decode(token);

            auto verifier = jwt::verify()
                                .allow_algorithm(jwt::algorithm::hs256{secretKey})
                                .with_issuer("uber_backend");

            verifier.verify(decoded);
            userIdOut = decoded.get_subject();
            return true;
        }
        catch (const std::exception &e)
        {
            std::cerr << "JWT verification failed: " << e.what() << std::endl;
            return false;
        }
    }

}
