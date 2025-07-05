#include "../include/sharedgRPCServer.h"
#include "../../proto/location.grpc.pb.h"

#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>

namespace UberBackend
{
    using grpc::Server;
    using grpc::ServerBuilder;
    using grpc::ServerContext;
    using grpc::Status;

    class LocationServiceImpl final : public UberBackend::LocationService::Service
    {
    public:
        Status SendLocation(ServerContext *context, const UberBackend::UserLocation *request, UberBackend::LocationAck *response) override
        {
            std::cout << "Received location for user " << request->user_id() << ": "
                      << request->latitude() << ", " << request->longitude() << std::endl;

            response->set_message("Location received");
            return Status::OK;
        }

        // Optionally implement GetNearbyUsers here
    };

    SharedgPRCServer::SharedgPRCServer(const std::string &server_address)
        : serverAddress(server_address),
          logger_(utils::SingletonLogger::instance())
    {
    }

    SharedgPRCServer::~SharedgPRCServer()
    {
        if (grpcServer)
        {
            grpcServer->Shutdown();
            logger_.logMeta(utils::SingletonLogger::INFO, "gRPC server shut down", __FILE__, __LINE__, __func__);
        }
    }

    void SharedgPRCServer::Run()
    {
        LocationServiceImpl service;

        ServerBuilder builder;
        builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);

        grpcServer = builder.BuildAndStart();
        logger_.logMeta(utils::SingletonLogger::INFO, "gRPC server started on " + serverAddress, __FILE__, __LINE__, __func__);

        grpcServer->Wait();
    }
}
