cmake_minimum_required(VERSION 3.25.2)

project(UserManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Define Dependencies ---
set(DEPENDENCIES
    app_utils::app_utils
    app_kafka::app_kafka
    app_h3::app_h3
    h3::h3
    app_database::app_database
    app_algorithms::app_algorithms
    gRPC::grpc++
    gRPC::grpc
    gRPC::grpc++_reflection
    protobuf::libprotobuf
    protobuf::libprotoc  # Optional: Only if you use protoc-related C++ APIs
    absl::base
    absl::synchronization
    absl::strings
    absl::time
    absl::str_format
    absl::status
    absl::random_random
    absl::hash
)

# --- Packages to Find ---
set(PACKAGES
    app_utils
    app_kafka
    app_h3
    h3
    app_database
    app_algorithms
    protobuf
    gRPC
    absl
)

# --- Find Packages ---
foreach(pack ${PACKAGES})
    find_package(${pack} CONFIG REQUIRED)
endforeach()

# --- Source Files ---
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE SHARED_RESOURCES CONFIGURE_DEPENDS ../sharedResources/src/*.cpp)
file(GLOB_RECURSE PROTO_SRCS CONFIGURE_DEPENDS ../proto/*.pb.cc ../proto/*.grpc.pb.cc)
file(GLOB_RECURSE PROTO_HDRS CONFIGURE_DEPENDS ../proto/*.pb.h ../proto/*.grpc.pb.h)

# --- Executable ---
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${SHARED_RESOURCES}
    ${PROTO_SRCS}
)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ../sharedResources/include
        ../proto
)

# --- Link Dependencies ---
target_link_libraries(${PROJECT_NAME}
    PRIVATE ${DEPENDENCIES}
)
