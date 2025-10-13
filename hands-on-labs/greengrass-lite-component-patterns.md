// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Greengrass Lite Component Patterns and Templates

## CRITICAL: IoT Core Publishing Requirements

**ALWAYS use Greengrass IPC ClientV2 for IoT Core publishing - NEVER use boto3 directly**

### Key Requirements for IoT Core Publishing:
1. **Use GreengrassCoreIPCClientV2** from awsiotsdk package
2. **QoS must be numeric** (0, 1, 2) - NOT string values like "AT_LEAST_ONCE"
3. **Access control** must be under `ComponentConfiguration.DefaultConfiguration`
4. **Use simplified pattern** for containerized environments (no install phase)

## Recipe Patterns

### Basic Recipe Structure
All Greengrass Lite component recipes follow this structure:
- RecipeFormatVersion: 2020-01-25
- ComponentName: Unique name (use timestamps to avoid conflicts)
- ComponentVersion: Semantic version (e.g., 1.0.0)
- ComponentDescription: Brief description of component purpose
- ComponentPublisher: Publisher name
- ComponentConfiguration: Optional configuration and access control
- Manifests: Platform specification and lifecycle definition
- Artifacts: Location of component files

### Platform Specification
Always use for Greengrass Lite:
```yaml
Platform:
  os: linux
  runtime: aws_nucleus_lite
```

### Access Control Pattern (for IoT Core Publishing)
```yaml
ComponentConfiguration:
  DefaultConfiguration:
    accessControl:
      aws.greengrass.ipc.mqttproxy:
        "{ComponentName}:mqttproxy:1":
          policyDescription: "Allows access to publish to IoT Core"
          operations:
            - "aws.greengrass#PublishToIoTCore"
          resources:
            - "topic/name/pattern"
```

### Lifecycle Patterns

**Simplified Pattern (Recommended for Containers):**
```yaml
Lifecycle:
  run: |
    export HOME=/tmp/gg-home
    python3 -u {artifacts:path}/script.py
```

**Traditional Pattern (with Dependencies):**
```yaml
Lifecycle:
  install: |
    mkdir -p /tmp/gg-home/.cache/pip
    export HOME=/tmp/gg-home
    python3 -m pip install package --break-system-packages
  run: |
    export HOME=/tmp/gg-home
    python3 -u {artifacts:path}/script.py
```

**Multi-Phase Pattern:**
```yaml
Lifecycle:
  install: |
    # Setup commands
  run: |
    # Main execution
  shutdown: |
    # Cleanup commands
```

### Artifact Patterns

**S3 Artifacts (Cloud Deployment):**
```yaml
Artifacts:
  - URI: s3://bucket-name/component-name/version/script.py
```

**Local Artifacts (Local Deployment):**
```yaml
Artifacts:
  - URI: file:///var/lib/greengrass/packages/artifacts/component-name/version/script.py
```

## Python Script Patterns

### Essential Script Elements
All component scripts should include:
- Proper shebang: `#!/usr/bin/env python3`
- Signal handling for graceful shutdown (SIGTERM, SIGINT)
- Error handling with try/catch blocks
- Logging with `flush=True` for proper systemd output
- Main execution guard: `if __name__ == "__main__":`

### IoT Core Publishing Pattern
For components that publish to IoT Core:
- Import: `from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2`
- Initialize: `ipc_client = GreengrassCoreIPCClientV2()`
- Publish: `ipc_client.publish_to_iot_core(topic_name=topic, payload=data, qos=1)`
- Use numeric QoS values (0, 1, 2) only

### Configuration Access Pattern
Components can access configuration via environment variables or IPC calls based on recipe configuration.

## Deployment Patterns

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

## Best Practices

### Recipe Development
- Use unique component names with timestamps
- Use `runtime: aws_nucleus_lite` in Platform section
- Use lowercase lifecycle keys (`run`, `install`, `shutdown`)
- Include proper artifact URI with full path
- Add meaningful ComponentDescription

### Python Script Development
- Handle SIGTERM for graceful shutdown
- Use `flush=True` in all print statements
- Include error handling with try/catch blocks
- Set proper HOME directory for pip operations
- Use proper logging instead of print for production

### Access Control
- Place access control under ComponentConfiguration.DefaultConfiguration
- Match resources to actual topics/operations used
- Use descriptive policy descriptions
- Follow principle of least privilege

### Troubleshooting
- Test scripts manually before deployment
- Verify service status after deployment
- Monitor logs for successful startup
- Check file ownership and permissions
- Ensure dependencies are available

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

## Troubleshooting Patterns

### Common Component Failures

**"ModuleNotFoundError: No module named 'awsiot'"**
- Cause: Missing awsiotsdk dependency
- Solution: Add install phase to lifecycle:
```yaml
Lifecycle:
  install: |
    mkdir -p /tmp/gg-home
    export HOME=/tmp/gg-home
    python3 -m pip install awsiotsdk --break-system-packages
```

**"Failed to chmod server socket: 22"**
- Cause: /run/greengrass ownership issues
- Solution: `chown ggcore:ggcore /run/greengrass`

**"Failed to connect to server (/run/greengrass/gg_config): 111"**
- Cause: ggconfigd service not running
- Solution: Enable and start all 8 core services

**Devices not appearing in Greengrass console**
- Cause: gg-fleet-statusd service not running
- Solution: Enable ggl.core.gg-fleet-statusd.service

### Debugging Commands
- Component logs: `journalctl -u ggl.{ComponentName}.service --no-pager -n 10`
- Deployment logs: `journalctl -u ggl.core.ggdeploymentd.service --no-pager -n 10`
- Service status: `systemctl is-active ggl.core.*.service`
- All services: `systemctl status ggl.core.* --no-pager`

This pattern-based approach allows AI agents to create appropriate components for any use case while following Greengrass Lite best practices.
