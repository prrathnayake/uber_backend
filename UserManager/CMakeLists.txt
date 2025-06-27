cmake_minimum_required(VERSION 3.25.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(UserManager)

# Dependencies this target needs
set(DEPENDENCIES
    app_utils::app_utils
    app_kafka::app_kafka
    app_h3::app_h3
    h3::h3
    app_database::app_database
    app_algorithms::app_algorithms
)

# Packages we need to find
set(PACKAGES
    app_utils
    app_kafka
    app_h3
    h3
    app_database
    app_algorithms
)

# Find required packages
foreach(pack ${PACKAGES})
    find_package(${pack} CONFIG REQUIRED)
endforeach()

# Collect source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)


# Include directories
include_directories(include)

# Create executable target
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE ${DEPENDENCIES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)