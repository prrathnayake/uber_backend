#include <iostream>

#include <utils/index.h>

int main()
{
    auto &logger = utils::SingletonLogger::instance();
    logger.logMeta(utils::SingletonLogger::INFO,
                   "Starting Ride Manager stub server (in-memory mode)",
                   __FILE__,
                   __LINE__,
                   __func__);

    std::cout << "RideManager service initialised with stubbed dependencies." << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    logger.logMeta(utils::SingletonLogger::INFO,
                   "Shutting down Ride Manager stub server",
                   __FILE__,
                   __LINE__,
                   __func__);
    return 0;
}
