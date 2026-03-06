# AWS Greengrass V2 C Component Examples

This directory contains C examples for migrating V1 Lambda functions to V2 components.

## Prerequisites

To build the SDK and components, you need:

- GCC or Clang
- CMake (at least version 3.22)
- Make or Ninja

### Install Build Dependencies

**On Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake
```

**On Amazon Linux:**
```bash
sudo yum install gcc cmake make
```

## Building Components

### 1. Clone the AWS Greengrass Component SDK

```bash
# Clone into your component directory
git clone https://github.com/aws-greengrass/aws-greengrass-component-sdk.git
```

### 2. Create CMakeLists.txt

Example `CMakeLists.txt` for a component:

```cmake
cmake_minimum_required(VERSION 3.10)
project(TemperatureProcessor C)

set(CMAKE_C_STANDARD 11)

# Add AWS Greengrass Component SDK
add_subdirectory(aws-greengrass-component-sdk)

# Build your component executable
add_executable(temperature_processor temperature_processor.c)
target_link_libraries(temperature_processor gg-sdk)
```

### 3. Build

```bash
# Build your component
cmake -B build -D CMAKE_BUILD_TYPE=MinSizeRel
make -C build -j$(nproc)

# The binary will be in ./build/
```

### 4. Deploy

Upload the binary to S3 or use local deployment with the binary path in your recipe.

## Examples

- **local_communication/**: Examples using local pub/sub (PublishToTopic, SubscribeToTopic)
- **cloud_communication/**: Examples using cloud MQTT (PublishToIoTCore, SubscribeToIoTCore)

Each example demonstrates the V2.1 SDK patterns with proper initialization and IPC operations.
