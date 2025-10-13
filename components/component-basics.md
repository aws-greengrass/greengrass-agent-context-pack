// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Component Basics

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
      "Lifecycle": {
        "Run": "python3 {artifacts:path}/main.py"
      }
    }
  ]
}
```

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
