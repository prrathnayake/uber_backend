cmake_minimum_required(VERSION 3.25.2)

project(uberBackend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake OPTIONAL)

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
