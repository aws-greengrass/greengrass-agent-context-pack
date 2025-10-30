# Greengrass Component Development Guide

This directory contains guides and templates for developing AWS IoT Greengrass components.

## Component Recipe Template

```json
{
  "RecipeFormatVersion": "2020-01-25",
  "ComponentName": "com.example.MyComponent",
  "ComponentVersion": "1.0.0",
  "ComponentDescription": "My custom component",
  "ComponentPublisher": "Amazon",
  "ComponentConfiguration": {
    "DefaultConfiguration": {
      "message": "Hello World"
    }
  },
  "Manifests": [
    {
      "Platform": {
        "os": "linux"
      },
      "runtime": "*",
      "lifecycle": {
        "run": "python3 {artifacts:path}/main.py"
      }
    }
  ]
}
```

## CRITICAL: Recipe Field Case Sensitivity

Greengrass Lite is CASE-SENSITIVE for ALL recipe fields. Use exact casing as specified in AWS documentation:

- `"Platform"` (not "platform")
- `"runtime"` (not "Runtime") 
- `"Lifecycle"` (not "lifecycle")
- `"Artifacts"` (not "artifacts")
- `"run"`, `"install"`, `"startup"` (not "Run", "Install", "Startup")
- `"Uri"` (not "URI")
- `"Unarchive"` (not "unarchive")

**For clarification look at the AWS greengrass recipe reference page: https://docs.aws.amazon.com/greengrass/v2/developerguide/component-recipe-reference.html**

## Runtime Field Requirements

- **Universal compatibility**: `"runtime": "*"` (works on both Greengrass (full runtime) and Lite (constrained devices))
- **Greengrass Lite only**: `"runtime": "aws_nucleus_lite"`
- **Greengrass only**: Omit runtime field entirely
- **Default recommendation**: Use `"runtime": "*"` for maximum compatibility unless specific runtime targeting is required

## Component Creation Workflow

When creating components, follow this specific workflow:

1. **IDENTIFY SIMILAR EXAMPLE** - Check `../examples/` directory for the closest matching component type
2. **COPY AND MODIFY** - Use the example as a starting template, don't create from scratch
3. **UPDATE CONFIGURATION** - Modify recipe.json with user's specific requirements
4. **ADAPT SOURCE CODE** - Modify the source code for user's specific functionality


## Runtime Options

### Full Nucleus vs Lite
- **Greengrass Nucleus**: Full-featured runtime with complete IPC support
- **Greengrass Lite**: Lightweight runtime for constrained devices with subset of features

### Choosing the Right Runtime
- Use **Nucleus** for full-featured applications requiring complete IPC support
- Use **Lite** for resource-constrained devices (IoT edge devices, embedded systems)

## Component Lifecycle
1. **Install**: Component artifacts downloaded and prepared
2. **Run**: Component process started and monitored
3. **Shutdown**: Graceful component termination

## Accessing AWS Resources
- When publishing MQTT messages to IoT core use IPC. See the IPC communication section below.
- When accessing AWS resources TES is required. Refer to the guide here: `./token-exchange-service-guide.md`

## Service Dependencies
Components can depend on other services:
- Dependencies must start before dependent services
- Automatic restart when dependencies become unstable
- Configuration changes propagate to dependent services

## IPC Communication
Available IPC operations (varies by runtime):
- Topic publish/subscribe (local and IoT Core)
- Configuration management
- Local deployments
- Component lifecycle control

## Best Practices
- Define clear component dependencies
- Handle configuration updates gracefully
- Implement proper shutdown procedures
- Use appropriate runtime for device constraints
- Test components in isolation before deployment
- **Always reference working examples** from `../examples/` directory
- **Follow exact field casing** requirements for Greengrass Lite compatibility

## Available Guides

- `token-exchange-service-guide.md` - Critical TES dependency requirements for AWS service access
- `greengrass-lite-component-patterns.md` - Patterns to use in greengrass-lite components

## Working Examples

For complete, tested component implementations, see the `../examples/` directory which includes:
- hello-world - Simple logging component
- sensor-simulator - Multi-sensor simulation with realistic data
- iot-core-publisher - Publishes sensor data to AWS IoT Core
- s3-uploader - Monitors directories and uploads files to S3
- ipc-publisher/subscriber - Inter-component communication examples
