cmake_minimum_required(VERSION 3.25.2)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(uber_shared_resources
    src/sharedDatabase.cpp
    src/sharedHTTPClient.cpp
    src/sharedHTTPHandler.cpp
    src/sharedHTTPServer.cpp
    src/sharedKafkaConsumer.cpp
    src/sharedKafkaHandler.cpp
    src/sharedKafkaProducer.cpp
    src/sharedRabbitMQConsumer.cpp
    src/sharedRabbitMQHandler.cpp
    src/sharedRabbitMQProducer.cpp
    src/sharedRouteHandler.cpp
    src/sharedServer.cpp
    src/sharedgRPCClienth.cpp
    src/sharedgRPCServer.cpp
    src/database/MySQLDatabase.cpp
    src/kafka/KafkaStub.cpp
    src/h3/H3Stub.cpp
)

target_include_directories(uber_shared_resources
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/database
        ${CMAKE_CURRENT_SOURCE_DIR}/include/kafka
        ${CMAKE_CURRENT_SOURCE_DIR}/include/h3
        ${CMAKE_CURRENT_SOURCE_DIR}/../sharedUtils/include
)

target_link_libraries(uber_shared_resources
    PUBLIC
        uber_shared_utils
        nlohmann_json::nlohmann_json
)

add_library(app_utils INTERFACE)
target_link_libraries(app_utils INTERFACE uber_shared_utils)
add_library(app_utils::app_utils ALIAS app_utils)

foreach(component IN ITEMS database kafka rabbitmq h3 blockchain algorithms)
    add_library(app_${component} INTERFACE)
    target_link_libraries(app_${component} INTERFACE uber_shared_resources)
    add_library(app_${component}::app_${component} ALIAS app_${component})
endforeach()

if(NOT TARGET h3::h3)
    add_library(h3_stub INTERFACE)
    target_link_libraries(h3_stub INTERFACE uber_shared_resources)
    target_include_directories(h3_stub INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/h3)
    add_library(h3::h3 ALIAS h3_stub)
endif()

