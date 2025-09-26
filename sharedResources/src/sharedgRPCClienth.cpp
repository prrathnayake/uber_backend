#include "../include/sharedgRPCClient.h"

namespace UberBackend
{

    LocationClient::LocationClient(const std::string &endpoint)
        : logger_(utils::SingletonLogger::instance()),
          endpoint_(endpoint)
    {
        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Initialized simulated gRPC client for " + endpoint_,
                        __FILE__,
                        __LINE__,
                        __func__);
    }

    std::string LocationClient::SendLocation(const std::string &userID, double lat, double lon)
    {
        logger_.logMeta(utils::SingletonLogger::INFO,
                        "Simulated SendLocation -> endpoint=" + endpoint_ +
                            ", user=" + userID +
                            ", lat=" + std::to_string(lat) +
                            ", lon=" + std::to_string(lon),
                        __FILE__,
                        __LINE__,
                        __func__);
        return "Location acknowledged";
    }

}
