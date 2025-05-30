cmake_minimum_required(VERSION 3.22.1)
project(monitor_receiver)

file(GLOB SRCS ./*cpp)
message("SRCS: ${SRCS}")

add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE 3rd/include)
message("platform " ${PLATFORM})
if (${PLATFORM} STREQUAL "arm")
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)
    target_link_directories(${PROJECT_NAME} PRIVATE 3rd/lib/arm)
else()
    target_link_directories(${PROJECT_NAME} PRIVATE 3rd/lib/x86)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE paho-embed-mqtt3cc)
target_compile_definitions(${PROJECT_NAME} PRIVATE MQTTCLIENT_PLATFORM_HEADER=MQTTLinux.h)