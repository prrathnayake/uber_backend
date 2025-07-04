#include "locationClient.h"
#include <iostream>

using namespace UberBackend;
using namespace utils;

LocationClient::LocationClient(const std::shared_ptr<grpc::Channel> &channel)
    : stub_(Location::NewStub(channel)), logger_(SingletonLogger::instance()), {}

std::string LocationClient::SendLocation(const std::string &userID, double lat, double lon)
{
    LocationRequest request;
    request.set_userid(userID);
    request.set_latitude(lat);
    request.set_longitude(lon);

    LocationResponse response;
    grpc::ClientContext context;

    grpc::Status status = stub_->SendLocation(&context, request, &response);

    if (status.ok())
    {
        return response.message();
    }
    else
    {
        std::cerr << "gRPC Error: " << status.error_code() << ": " << status.error_message() << std::endl;
        return "gRPC call failed";
    }
}
