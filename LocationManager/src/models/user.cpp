#include "../../include/models/user.h"

namespace UberBackend
{

  User::User()
      : logger_(SingletonLogger::instance()) {}

  User::User(const std::string &firstName,
             const std::string &middleName,
             const std::string &lastName,
             const std::string &countryCode,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash,
             const std::string &role,
             const std::string &preferredLanguage,
             const std::string &currency,
             const std::string &country)
      : firstName_(firstName),
        middleName_(middleName),
        lastName_(lastName),
        mobileNumber_(mobileNumber),
        address_(address),
        email_(email),
        username_(username),
        role_(role),
        passwordHash_(passwordHash),
        logger_(SingletonLogger::instance())
  {
  }

  User::~User() = default;

  std::string User::getFirstName() const { return firstName_; }
  std::string User::getMiddleName() const { return middleName_; }
  std::string User::getLastName() const { return lastName_; }
  std::string User::getMobileNumber() const { return mobileNumber_; }
  std::string User::getAddress() const { return address_; }
  std::string User::getEmail() const { return email_; }
  std::string User::getUsername() const { return username_; }
  std::string User::getRole() const { return role_; }
  std::string User::getPasswordHash() const { return passwordHash_; }
  std::string User::getCountryCode() const { return countryCode_; }
  std::string User::getPreferredLanguage() const { return preferredLanguage_; }
  std::string User::getCurrency() const { return currency_; }
  std::string User::getCountry() const { return country_; }

  void User::setFirstName(const std::string &firstName) { firstName_ = firstName; }
  void User::setMiddleName(const std::string &middleName) { middleName_ = middleName; }
  void User::setLastName(const std::string &lastName) { lastName_ = lastName; }
  void User::setMobileNumber(const std::string &mobileNumber) { mobileNumber_ = mobileNumber; }
  void User::setAddress(const std::string &address) { address_ = address; }
  void User::setEmail(const std::string &email) { email_ = email; }
  void User::setUsername(const std::string &username) { username_ = username; }
  void User::setRole(const std::string &role) { role_ = role; }
  void User::setPasswordHash(const std::string &hash) { passwordHash_ = hash; }
  void User::setCountryCode(const std::string &countryCode) { countryCode_ = countryCode; }
  void User::setPreferredLanguage(const std::string &language) { preferredLanguage_ = language; }
  void User::setCurrency(const std::string &currency) { currency_ = currency; }
  void User::setCountry(const std::string &country) { country_ = country; }

}
