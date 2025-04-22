cmake_minimum_required(VERSION 3.22.1)
project(monitor_client)

file(GLOB_RECURSE SRCS "src/*cpp")
message("src: ${SRCS}")
add_executable(${PROJECT_NAME} ${SRCS})
target_link_directories(${PROJECT_NAME} PRIVATE 3rd/lib/x86)
target_include_directories(${PROJECT_NAME} PRIVATE 3rd/include include)

target_link_libraries(${PROJECT_NAME} PRIVATE paho-embed-mqtt3cc opencv_core opencv_highgui opencv_videoio pthread)
target_compile_definitions(${PROJECT_NAME} PRIVATE MQTTCLIENT_PLATFORM_HEADER=MQTTLinux.h)