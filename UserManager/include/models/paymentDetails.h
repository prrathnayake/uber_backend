#pragma once

#include <string>
#include <iostream>
#include <utils/index.h>

using namespace utils;

namespace UberBackend
{
  class PaymentDetails
  {
  public:
    PaymentDetails();
    PaymentDetails(const std::string &userId,
                   const std::string &paymentType,
                   const std::string &cardNumberMasked,
                   const std::string &expiryDate,
                   const std::string &provider,
                   bool isDefault);

    ~PaymentDetails();

    std::string getUserId() const;
    std::string getPaymentType() const;
    std::string getCardNumberMasked() const;
    std::string getExpiryDate() const;
    std::string getProvider() const;
    bool isDefaultMethod() const;

    void setUserId(const std::string &id);
    void setPaymentType(const std::string &type);
    void setCardNumberMasked(const std::string &masked);
    void setExpiryDate(const std::string &expiry);
    void setProvider(const std::string &provider);
    void setIsDefault(bool value);

  private:
    SingletonLogger &logger_;

    std::string userId_;
    std::string paymentType_;
    std::string cardNumberMasked_;
    std::string expiryDate_;
    std::string provider_;
    bool isDefault_;
  };
}
