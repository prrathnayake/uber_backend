cmake_minimum_required(VERSION 3.25.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(RideManager)

if(NOT TARGET nlohmann_json::nlohmann_json)
    find_package(nlohmann_json CONFIG REQUIRED)
endif()

# Collect source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

# Create executable target
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link dependencies
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        uber_shared_resources
)
