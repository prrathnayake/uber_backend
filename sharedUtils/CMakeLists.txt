cmake_minimum_required(VERSION 3.25.2)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(uber_shared_utils
    src/config.cpp
    src/jwt.cpp
    src/utils.cpp
    src/algorithms_sha256.cpp
)

target_include_directories(uber_shared_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(uber_shared_utils
    PUBLIC
        nlohmann_json::nlohmann_json
)

