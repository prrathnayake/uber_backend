#pragma once
#include <string>
#include <iostream>

#include <database/index.h>
#include <utils/index.h>

namespace uber_backend
{
    class User
    {
    public:
        // Constructors
        User();
        User(const std::string &firstName,
             const std::string &middleName,
             const std::string &lastName,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash);

        // Destructor
        ~User();

        // Public data members (or use getters/setters)
        std::string firstName;
        std::string middleName;
        std::string lastName;
        std::string mobileNumber;
        std::string address;
        std::string email;
        std::string username;

        // Getters
        std::string getPasswordHash() const;

        // Setters
        void setPasswordHash(const std::string &hash);

    private:
        std::string passwordHash;
    };

} // namespace uber_backend
