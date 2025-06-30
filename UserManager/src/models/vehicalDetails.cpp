#include "../../../include/models/user.h"

using namespace uber_backend;
using namespace utils;

VehicleDetails::VehicleDetails()
    : logger_(SingletonLogger::instance()),
      registrationYear_(0) {}

VehicleDetails::VehicleDetails(const std::string &userId,
                               const std::string &vehicleType,
                               const std::string &make,
                               const std::string &model,
                               const std::string &licensePlate,
                               int registrationYear,
                               const std::string &color)
    : logger_(SingletonLogger::instance()),
      userId_(userId),
      vehicleType_(vehicleType),
      make_(make),
      model_(model),
      licensePlate_(licensePlate),
      registrationYear_(registrationYear),
      color_(color)
{
}

VehicleDetails::~VehicleDetails() = default;

std::string VehicleDetails::getUserId() const { return userId_; }
std::string VehicleDetails::getVehicleType() const { return vehicleType_; }
std::string VehicleDetails::getMake() const { return make_; }
std::string VehicleDetails::getModel() const { return model_; }
std::string VehicleDetails::getLicensePlate() const { return licensePlate_; }
int VehicleDetails::getRegistrationYear() const { return registrationYear_; }
std::string VehicleDetails::getColor() const { return color_; }

// Setters
void VehicleDetails::setUserId(const std::string &userId) { userId_ = userId; }
void VehicleDetails::setVehicleType(const std::string &type) { vehicleType_ = type; }
void VehicleDetails::setMake(const std::string &make) { make_ = make; }
void VehicleDetails::setModel(const std::string &model) { model_ = model; }
void VehicleDetails::setLicensePlate(const std::string &plate) { licensePlate_ = plate; }
void VehicleDetails::setRegistrationYear(int year) { registrationYear_ = year; }
void VehicleDetails::setColor(const std::string &color) { color_ = color; }
