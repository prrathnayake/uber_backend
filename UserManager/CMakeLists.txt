cmake_minimum_required(VERSION 3.25.2)

project(UserManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(nlohmann_json CONFIG REQUIRED)

# --- Source Files ---
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
# --- Executable ---
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
)

# --- Include Directories ---
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ../sharedResources/include
        ../sharedUtils/include
)

# --- Link Dependencies ---
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        uber_shared_resources
)
