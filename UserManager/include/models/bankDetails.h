#pragma once

#include <string>
#include <iostream>

#include <utils/index.h>

namespace uber_backend
{
    class BankDetails
    {
    public:
        BankDetails();
        BankDetails(const std::string &userId,
                    const std::string &bankName,
                    const std::string &accountHolderName,
                    const std::string &accountNumber,
                    const std::string &bsbCode,
                    const std::string &bankCountry);

        ~BankDetails();

        
        std::string getUserId() const;
        std::string getBankName() const;
        std::string getAccountHolderName() const;
        std::string getAccountNumber() const;
        std::string getBsbCode() const;
        std::string getBankCountry() const;

        
        void setUserId(const std::string &userId);
        void setBankName(const std::string &bankName);
        void setAccountHolderName(const std::string &name);
        void setAccountNumber(const std::string &accountNumber);
        void setBsbCode(const std::string &bsbCode);
        void setBankCountry(const std::string &country);

    private:
        SingletonLogger &logger_;

        std::string userId_;           
        std::string bankName_;            
        std::string accountHolderName_;   
        std::string accountNumber_;       
        std::string bsbCode_;             
        std::string bankCountry_;         
    };
}
