// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Deployment Patterns

## Basic preferences
### Deployment Strategy Preference
**Default to Cloud Deployment** when not specified by user:
- More reliable than local deployment for Greengrass Lite
- Requires S3 permissions on Token Exchange Role

### Local Deployment (Development)
- Copy recipe to `/var/lib/greengrass/packages/recipes/`
- Copy artifacts to `/var/lib/greengrass/packages/artifacts/`
- Set proper ownership (ggcore:ggcore)
- Deploy using `ggl-cli deploy --add-component`

### Cloud Deployment (Production)
- Upload artifacts to S3
- Create component version via AWS Greengrass service
- Create thing group and add device
- Deploy to thing group (required for Greengrass Lite)

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

## Deployment Requirements

### Thing Groups Mandatory
**Greengrass Lite devices MUST be in thing groups for cloud deployments**
- Create thing group before deployment
- Add device to thing group
- Deploy to thing group ARN, not individual device

### Component Version Management
- Version conflicts require empty deployment to remove old versions
- Deploy empty components `{}` to remove all components from target
- Then deploy new versions to avoid conflicts

### Recipe Format Considerations
- JSON recipes require base64 encoding for AWS CLI: `base64 -i recipe.json`
- YAML recipes work better for local development and file-based operations
- Use `aws greengrassv2 create-component-version --inline-recipe "$RECIPE_B64"`

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
