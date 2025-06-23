cmake_minimum_required(VERSION 3.25.2)

set(CMAKE_CXX_STANDARD 11)

project(uber)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(DEPENDENCIES
     app_h3::app_h3
     app_kafka::app_kafka
     app_utils::app_utils
     h3::h3
     app_database::app_database
)

set(PACKAGES
     app_utils
     app_kafka
     app_h3
     h3
     app_database
)

foreach(pack ${PACKAGES})
     find_package(${pack} CONFIG REQUIRED)
endforeach()  

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

foreach(dep ${DEPENDENCIES})
     target_link_libraries(${PROJECT_NAME} ${dep})
endforeach()   
 