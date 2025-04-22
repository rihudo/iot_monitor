cmake_minimum_required(VERSION 3.22.1)
project(monitor_receiver)

file(GLOB SRCS ./*cpp)
message("SRCS: ${SRCS}")

add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE 3rd/include)
target_link_directories(${PROJECT_NAME} PRIVATE 3rd/lib/x86)
target_link_libraries(${PROJECT_NAME} PRIVATE paho-embed-mqtt3cc)
target_compile_definitions(${PROJECT_NAME} PRIVATE MQTTCLIENT_PLATFORM_HEADER=MQTTLinux.h)