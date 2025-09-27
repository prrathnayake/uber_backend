cmake_minimum_required(VERSION 3.25.2)

if(NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json CONFIG REQUIRED)
endif()

if(NOT CCP_BASE_SOURCE_DIR)
    message(FATAL_ERROR "CCP_BASE_SOURCE_DIR is not defined. Did you forget to populate cpp-base?")
endif()

set(CCP_BASE_UTILS_DIR "${CCP_BASE_SOURCE_DIR}/app_utils/src")
set(CCP_BASE_ALG_DIR "${CCP_BASE_SOURCE_DIR}/app_algorithms/src")

add_library(uber_shared_utils
    src/config.cpp
    src/jwt.cpp
    ${CCP_BASE_UTILS_DIR}/utils/log/consoleLogger.cpp
    ${CCP_BASE_UTILS_DIR}/utils/log/fileLogger.cpp
    ${CCP_BASE_UTILS_DIR}/utils/log/singletonLogger.cpp
    ${CCP_BASE_UTILS_DIR}/utils/thread/ThreadPool.cpp
    ${CCP_BASE_UTILS_DIR}/utils/chrono/time.cpp
    ${CCP_BASE_UTILS_DIR}/utils/chrono/latency.cpp
    ${CCP_BASE_ALG_DIR}/algorithms/sha256/common.cpp
    ${CCP_BASE_ALG_DIR}/algorithms/sha256/conversions.cpp
    ${CCP_BASE_ALG_DIR}/algorithms/sha256/formulas.cpp
    ${CCP_BASE_ALG_DIR}/algorithms/sha256/sha256.cpp
    ${CCP_BASE_ALG_DIR}/algorithms/sha256/binaryOperatiors.cpp
)

list(APPEND CCP_BASE_UTILS_PUBLIC_INCLUDE_DIRS
    ${CCP_BASE_UTILS_DIR}
    ${CCP_BASE_ALG_DIR}
)

list(REMOVE_DUPLICATES CCP_BASE_UTILS_PUBLIC_INCLUDE_DIRS)

set_target_properties(uber_shared_utils PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(uber_shared_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CCP_BASE_UTILS_PUBLIC_INCLUDE_DIRS}
)

target_link_libraries(uber_shared_utils
    PUBLIC
        nlohmann_json::nlohmann_json
)
