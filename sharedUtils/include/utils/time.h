#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace utils {

class Time {
private:
    static std::string validTime(int value);

public:
    static std::uint64_t getEpocTimeInNanoseconds();
    static std::uint64_t getEpocTimeInMicroseconds();
    static std::uint64_t getEpocTimeInMilliseconds();
    static std::uint64_t getEpocTimeInSeconds();
    static std::uint64_t getEpocTimeInMinutes();
    static void printNowTime();
    static void holdSeconds(int seconds);
    static void holdMiliseconds(int milliseconds);
    static std::string logTime();
};

} // namespace utils

