#include "sharedgPRCServer.h"

#include <location.grpc.pb.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace UberBackend;
using namespace utils;

class LocationServiceImpl final : public Location::Service
{
    Status SendLocation(ServerContext *context, const LocationRequest *request, LocationResponse *response) override
    {
        std::cout << "Received location for user " << request->userid() << ": "
                  << request->latitude() << ", " << request->longitude() << std::endl;

        response->set_message("Location received");
        return Status::OK;
    }
};

SharedgPRCServer::SharedgPRCServer(const std::string &server_address)
    : serverAddress(server_address),
      logger_(SingletonLogger::instance())
{
}

SharedgPRCServer::~SharedgPRCServer()
{
    if (grpcServer)
        grpcServer->Shutdown();
}

void SharedgPRCServer::Run()
{
    LocationServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    grpcServer = builder.BuildAndStart();
    std::cout << "gRPC server listening on " << serverAddress << std::endl;
    grpcServer->Wait();
}
