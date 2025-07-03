#include "../../include/models/user.h"

namespace UberBackend
{

  User::User()
      : logger_(SingletonLogger::instance()) {}

  User::User(const std::string &firstName,
             const std::string &middleName,
             const std::string &lastName,
             const std::string &mobileNumber,
             const std::string &address,
             const std::string &email,
             const std::string &username,
             const std::string &passwordHash,
             const std::string &role)
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

  void User::setFirstName(const std::string &firstName) { firstName_ = firstName; }
  void User::setMiddleName(const std::string &middleName) { middleName_ = middleName; }
  void User::setLastName(const std::string &lastName) { lastName_ = lastName; }
  void User::setMobileNumber(const std::string &mobileNumber) { mobileNumber_ = mobileNumber; }
  void User::setAddress(const std::string &address) { address_ = address; }
  void User::setEmail(const std::string &email) { email_ = email; }
  void User::setUsername(const std::string &username) { username_ = username; }
  void User::setRole(const std::string &role) { role_ = role; }
  void User::setPasswordHash(const std::string &hash) { passwordHash_ = hash; }

}
