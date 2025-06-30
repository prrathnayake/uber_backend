#include "../../../include/models/bankDetails.h"

using namespace uber_backend;
using namespace utils;

BankDetails::BankDetails() = default;

BankDetails::BankDetails(const std::string &userId,
                         const std::string &bankName,
                         const std::string &accountHolderName,
                         const std::string &accountNumber,
                         const std::string &bsbCode,
                         const std::string &bankCountry)
    : userId_(userId),
      bankName_(bankName),
      accountHolderName_(accountHolderName),
      accountNumber_(accountNumber),
      bsbCode_(bsbCode),
      bankCountry_(bankCountry),
      logger_(SingletonLogger::instance())
{
}

BankDetails::~BankDetails() = default;

std::string BankDetails::getUserId() const { return userId_; }
std::string BankDetails::getBankName() const { return bankName_; }
std::string BankDetails::getAccountHolderName() const { return accountHolderName_; }
std::string BankDetails::getAccountNumber() const { return accountNumber_; }
std::string BankDetails::getBsbCode() const { return bsbCode_; }
std::string BankDetails::getBankCountry() const { return bankCountry_; }

void BankDetails::setUserId(const std::string &userId) { userId_ = userId; }
void BankDetails::setBankName(const std::string &bankName) { bankName_ = bankName; }
void BankDetails::setAccountHolderName(const std::string &name) { accountHolderName_ = name; }
void BankDetails::setAccountNumber(const std::string &accountNumber) { accountNumber_ = accountNumber; }
void BankDetails::setBsbCode(const std::string &bsbCode) { bsbCode_ = bsbCode; }
void BankDetails::setBankCountry(const std::string &country) { bankCountry_ = country; }
