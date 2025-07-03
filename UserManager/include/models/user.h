#pragma once

#include <string>
#include <iostream>

#include <utils/index.h>

#include "paymentDetails.h"
#include "bankDetails.h"

using namespace utils;

namespace UberBackend
{
    class User
    {
    public:
        User();

        User(const std::string &firstName,
             const std::string &middleName,
             const std::string &lastName,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash,
             const std::string &role);

        ~User();

        std::string getFirstName() const;
        std::string getMiddleName() const;
        std::string getLastName() const;
        std::string getMobileNumber() const;
        std::string getAddress() const;
        std::string getEmail() const;
        std::string getUsername() const;
        std::string getRole() const;
        std::string getPasswordHash() const;

        void setFirstName(const std::string &firstName);
        void setMiddleName(const std::string &middleName);
        void setLastName(const std::string &lastName);
        void setMobileNumber(const std::string &mobileNumber);
        void setAddress(const std::string &address);
        void setEmail(const std::string &email);
        void setUsername(const std::string &username);
        void setRole(const std::string &role);
        void setPasswordHash(const std::string &hash);

    private:
        SingletonLogger &logger_;

        std::string firstName_;
        std::string middleName_;
        std::string lastName_;
        std::string mobileNumber_;
        std::string address_;
        std::string email_;
        std::string username_;
        std::string role_ = "";
        std::string passwordHash_;
        PaymentDetails paymentDetails_;
        BankDetails bankDetails_;
    };
}
