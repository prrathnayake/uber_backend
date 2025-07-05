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
             const std::string &countryCode,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash,
             const std::string &role,
             const std::string &preferredLanguage = "en",
             const std::string &currency = "USD",
             const std::string &country = "");

        ~User();

        // Getters
        std::string getFirstName() const;
        std::string getMiddleName() const;
        std::string getLastName() const;
        std::string getCountryCode() const;
        std::string getMobileNumber() const;
        std::string getAddress() const;
        std::string getEmail() const;
        std::string getUsername() const;
        std::string getRole() const;
        std::string getPasswordHash() const;
        std::string getPreferredLanguage() const;
        std::string getCurrency() const;
        std::string getCountry() const;

        // Setters
        void setFirstName(const std::string &firstName);
        void setMiddleName(const std::string &middleName);
        void setLastName(const std::string &lastName);
        void setCountryCode(const std::string &countryCode);
        void setMobileNumber(const std::string &mobileNumber);
        void setAddress(const std::string &address);
        void setEmail(const std::string &email);
        void setUsername(const std::string &username);
        void setRole(const std::string &role);
        void setPasswordHash(const std::string &hash);
        void setPreferredLanguage(const std::string &language);
        void setCurrency(const std::string &currency);
        void setCountry(const std::string &country);

        PaymentDetails& getPaymentDetails();
        BankDetails& getBankDetails();

    private:
        SingletonLogger &logger_;

        std::string firstName_;
        std::string middleName_;
        std::string lastName_;
        std::string countryCode_;
        std::string mobileNumber_;
        std::string address_;
        std::string email_;
        std::string username_;
        std::string role_;
        std::string passwordHash_;
        std::string preferredLanguage_;
        std::string currency_;
        std::string country_;

        PaymentDetails paymentDetails_;
        BankDetails bankDetails_;
    };
}
