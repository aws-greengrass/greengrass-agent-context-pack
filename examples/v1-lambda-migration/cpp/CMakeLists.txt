cmake_minimum_required(VERSION 3.10)
project(GreengrassV2Examples CXX)

set(CMAKE_CXX_STANDARD 20)

# Add SDK as subdirectory
add_subdirectory(aws-greengrass-component-sdk)

# Add C++ SDK subdirectory
add_subdirectory(aws-greengrass-component-sdk/cpp)

# Local communication examples
add_executable(v2_sender local_communication/v2_sender.cpp)
target_link_libraries(v2_sender gg-sdk++)

add_executable(v2_receiver local_communication/v2_receiver.cpp)
target_link_libraries(v2_receiver gg-sdk++)

# Cloud communication example
add_executable(v2_cloud_publisher cloud_communication/v2_cloud_publisher.cpp)
target_link_libraries(v2_cloud_publisher gg-sdk++)
