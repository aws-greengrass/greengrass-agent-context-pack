// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Deployment Patterns

## Runtime Selection

### Full Nucleus Deployment
Best for:
- Devices with sufficient resources (>512MB RAM)
- Applications requiring full IPC feature set
- Complex component dependencies
- Production environments with complete monitoring needs

### Lite Runtime Deployment
Best for:
- Constrained devices (<256MB RAM)
- Simple component architectures
- Edge devices with limited resources
- Embedded systems and IoT sensors

## Deployment Strategies

### Local Development
```bash
# Create local deployment for testing
aws greengrassv2 create-deployment \
  --target-arn "arn:aws:iot:region:account:thing/device-name" \
  --components '{
    "com.example.MyComponent": {
      "componentVersion": "1.0.0"
    }
  }'
```

### Fleet Deployment
```bash
# Deploy to device group
aws greengrassv2 create-deployment \
  --target-arn "arn:aws:iot:region:account:thinggroup/my-fleet" \
  --deployment-policies '{
    "failureHandlingPolicy": "ROLLBACK",
    "componentUpdatePolicy": {
      "timeoutInSeconds": 300,
      "action": "NOTIFY_COMPONENTS"
    }
  }'
```

## Configuration Management

### Environment-Specific Configs
```json
{
  "ComponentConfiguration": {
    "DefaultConfiguration": {
      "environment": "production",
      "logLevel": "INFO"
    }
  }
}
```

### Runtime-Specific Considerations
- **Nucleus**: Full configuration validation and hot-reload
- **Lite**: Basic configuration support, limited validation

## Monitoring and Health Checks

### Component Health Monitoring
```json
{
  "Lifecycle": {
    "Run": "python3 main.py",
    "Shutdown": {
      "script": "cleanup.sh",
      "timeout": 30
    }
  }
}
```

### Deployment Rollback
- Automatic rollback on deployment failure
- Component-level rollback policies
- Health check integration

## Best Practices
- Test deployments in development environment first
- Use gradual rollout for fleet deployments
- Monitor component health during deployment
- Implement proper shutdown procedures
- Choose appropriate runtime for device constraints
