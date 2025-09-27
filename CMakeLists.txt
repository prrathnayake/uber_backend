cmake_minimum_required(VERSION 3.25.2)

project(uberBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

include(FetchContent)

# Ensure the cpp-base repository is available so we can reuse its
# foundational libraries instead of maintaining local duplicates.
FetchContent_Declare(
    cpp_base
    GIT_REPOSITORY https://github.com/prrathnayake/cpp-base.git
    GIT_TAG main
)

FetchContent_GetProperties(cpp_base)
if(NOT cpp_base_POPULATED)
    FetchContent_Populate(cpp_base)
endif()

set(CCP_BASE_SOURCE_DIR "${cpp_base_SOURCE_DIR}" CACHE PATH "cpp-base source directory")

# nlohmann_json is consumed as an imported target inside sharedUtils. The
# project previously expected the dependency to be provided by the host
# toolchain, which is not true in CI. Fetching it ensures consistent builds
# everywhere without requiring a pre-installed package.
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Output directories for all targets
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add subdirectories (these must each have their own CMakeLists.txt)
add_subdirectory(sharedUtils)
add_subdirectory(sharedResources)
add_subdirectory(UserManager)
add_subdirectory(RideManager)
add_subdirectory(LocationManager)
