#pragma once

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "location.grpc.pb.h"

#include <utils/index.h>

using namespace utils;

namespace UberBackend
{

    class LocationClient
    {
    public:
        explicit LocationClient(const std::shared_ptr<grpc::Channel> &channel);

        std::string SendLocation(const std::string &userID, double lat, double lon);

    private:
        SingletonLogger &logger_;
        std::unique_ptr<Location::Stub> stub_;
    };

}
