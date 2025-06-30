#pragma once

#include <string>
#include <iostream>
#include <utils/index.h>

using namespace utils;

namespace uber_backend
{
    class VehicleDetails
    {
    public:
        VehicleDetails();

        VehicleDetails(const std::string &userId,
                       const std::string &vehicleType,
                       const std::string &make,
                       const std::string &model,
                       const std::string &licensePlate,
                       int registrationYear,
                       const std::string &color);

        ~VehicleDetails();

        // Getters
        std::string getUserId() const;
        std::string getVehicleType() const;
        std::string getMake() const;
        std::string getModel() const;
        std::string getLicensePlate() const;
        int getRegistrationYear() const;
        std::string getColor() const;

        // Setters
        void setUserId(const std::string &userId);
        void setVehicleType(const std::string &type);
        void setMake(const std::string &make);
        void setModel(const std::string &model);
        void setLicensePlate(const std::string &plate);
        void setRegistrationYear(int year);
        void setColor(const std::string &color);

    private:
        SingletonLogger &logger_;

        std::string userId_;
        std::string vehicleType_;
        std::string make_;
        std::string model_;
        std::string licensePlate_;
        int registrationYear_;
        std::string color_;
    };
}
