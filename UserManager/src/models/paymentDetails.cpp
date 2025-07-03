#include "../../include/models/paymentDetails.h"

using namespace UberBackend;
using namespace utils;

PaymentDetails::PaymentDetails()
    : logger_(SingletonLogger::instance()) {}

PaymentDetails::PaymentDetails(const std::string &userId,
                               const std::string &paymentType,
                               const std::string &cardNumberMasked,
                               const std::string &expiryDate,
                               const std::string &provider,
                               bool isDefault)
    : userId_(userId),
      paymentType_(paymentType),
      cardNumberMasked_(cardNumberMasked),
      expiryDate_(expiryDate),
      provider_(provider),
      isDefault_(isDefault),
      logger_(SingletonLogger::instance())
{
}

PaymentDetails::~PaymentDetails() = default;

std::string PaymentDetails::getUserId() const { return userId_; }
std::string PaymentDetails::getPaymentType() const { return paymentType_; }
std::string PaymentDetails::getCardNumberMasked() const { return cardNumberMasked_; }
std::string PaymentDetails::getExpiryDate() const { return expiryDate_; }
std::string PaymentDetails::getProvider() const { return provider_; }
bool PaymentDetails::isDefaultMethod() const { return isDefault_; }

void PaymentDetails::setUserId(const std::string &id) { userId_ = id; }
void PaymentDetails::setPaymentType(const std::string &type) { paymentType_ = type; }
void PaymentDetails::setCardNumberMasked(const std::string &masked) { cardNumberMasked_ = masked; }
void PaymentDetails::setExpiryDate(const std::string &expiry) { expiryDate_ = expiry; }
void PaymentDetails::setProvider(const std::string &provider) { provider_ = provider; }
void PaymentDetails::setIsDefault(bool value) { isDefault_ = value; }
