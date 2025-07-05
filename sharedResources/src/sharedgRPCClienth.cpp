#include "../include/sharedgRPCClient.h"
#include <grpcpp/create_channel.h>
#include <chrono>

namespace UberBackend
{

    LocationClient::LocationClient(const std::shared_ptr<grpc::Channel> &channel)
        : stub_(UberBackend::LocationService::NewStub(channel)),
          logger_(utils::SingletonLogger::instance()) {}

    std::string LocationClient::SendLocation(const std::string &userID, double lat, double lon)
    {
        UberBackend::UserLocation request;
        request.set_user_id(userID);
        request.set_latitude(lat);
        request.set_longitude(lon);
        request.set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());

        UberBackend::LocationAck response;
        grpc::ClientContext context;

        grpc::Status status = stub_->SendLocation(&context, request, &response);

        if (status.ok())
        {
            logger_.logMeta(utils::SingletonLogger::INFO, "Location sent successfully: " + response.message(), __FILE__, __LINE__, __func__);
            return response.message();
        }
        else
        {
            logger_.logMeta(utils::SingletonLogger::ERROR, "gRPC SendLocation failed: " + status.error_message(), __FILE__, __LINE__, __func__);
            return "RPC failed";
        }
    }

}
