cmake_minimum_required(VERSION 3.10)
project(GreengrassV2Examples C)

set(CMAKE_C_STANDARD 11)

# Add AWS Greengrass Component SDK
add_subdirectory(aws-greengrass-component-sdk)

# Local communication examples
add_executable(v2_sender local_communication/v2_sender.c)
target_link_libraries(v2_sender gg-sdk)

add_executable(v2_receiver local_communication/v2_receiver.c)
target_link_libraries(v2_receiver gg-sdk)

# Cloud communication example
add_executable(v2_cloud_publisher cloud_communication/v2_cloud_publisher.c)
target_link_libraries(v2_cloud_publisher gg-sdk)
