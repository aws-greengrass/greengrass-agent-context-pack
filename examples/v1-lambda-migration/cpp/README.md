# AWS Greengrass V2 C++ Component Examples

This directory contains C++ examples for migrating V1 Lambda functions to V2 components.

## Prerequisites

To build the SDK and components, you need:

- GCC or Clang with C++20 support
- CMake (at least version 3.22)
- Make or Ninja

### Install Build Dependencies

**On Ubuntu/Debian:**
```bash
sudo apt install build-essential cmake
```

**On Amazon Linux:**
```bash
sudo yum install gcc-c++ cmake make
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
project(TemperatureProcessor CXX)

set(CMAKE_CXX_STANDARD 20)

# Add SDK as subdirectory
add_subdirectory(aws-greengrass-component-sdk)

# Add C++ SDK subdirectory
add_subdirectory(aws-greengrass-component-sdk/cpp)

add_executable(temperature_processor temperature_processor.cpp)
target_link_libraries(temperature_processor gg-sdk++)
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

- **local_communication/**: Examples using local pub/sub (publish_to_topic, subscribe_to_topic)
- **cloud_communication/**: Examples using cloud MQTT (publish_to_iot_core, subscribe_to_iot_core)

Each example demonstrates the V2.1 SDK patterns with:
- `gg::ipc::Client::get()` singleton pattern
- `client.connect()` initialization
- Callback classes inheriting from `LocalTopicCallback` or `IoTCoreTopicCallback`
- Modern C++ features (string_view, chrono, threads)
