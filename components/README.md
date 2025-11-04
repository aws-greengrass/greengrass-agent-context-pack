# Greengrass Component Development Guide

This directory contains guides and templates for developing AWS IoT Greengrass components.

## Component Recipe Template

When creating a recipe for any component, the agent *MUST* first read the `./comprehensice-component-recipe.yaml` *BEFORE* producing a recipe.

## CRITICAL: Recipe Field Case Sensitivity

Greengrass Lite is CASE-SENSITIVE for ALL recipe fields. Use exact casing as specified in AWS documentation:

- `"Platform"` (not "platform")
- `"runtime"` (not "Runtime") 
- `"Lifecycle"` (not "lifecycle")
- `"Artifacts"` (not "artifacts")
- `"run"`, `"install"`, `"startup"` (not "Run", "Install", "Startup")
- `"Uri"` (not "URI")
- `"Unarchive"` (not "unarchive")

**For clarification agent MUST read the AWS greengrass recipe reference page: https://docs.aws.amazon.com/greengrass/v2/developerguide/component-recipe-reference.html**

## Runtime Field Requirements

- **Universal compatibility**: `"runtime": "*"` (works on both Greengrass (full runtime) and Lite (constrained devices))
- **Greengrass Nucleus Lite only**: `"runtime": "aws_nucleus_lite"`
- **Greengrass Nucleus only**: Omit runtime field entirely
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
- **Greengrass Nucleus Lite**: Lightweight runtime for constrained devices with subset of features

### Choosing the Right Runtime
- Use **Greengrass Nucleus** for full-featured applications requiring complete IPC support
- Use **Greengrass Nucleus Lite** for resource-constrained devices (IoT edge devices, embedded systems)

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
- `comprehensive-component-recipe.yaml` - Recipe reference with all the fields supported 

## Working Examples

For complete, tested component implementations, see the `../examples/` directory which includes:
- hello-world - Simple logging component
- sensor-simulator - Multi-sensor simulation with realistic data
- iot-core-publisher - Publishes sensor data to AWS IoT Core
- s3-uploader - Monitors directories and uploads files to S3
- ipc-publisher/subscriber - Inter-component communication examples

### AWS Service Access Pattern
⚠️ **CRITICAL**: Components accessing AWS services (S3, DynamoDB, etc.) MUST declare TES dependency.
See: `components/token-exchange-service-guide.md`

For components that need AWS service access:
- Add ComponentDependencies for aws.greengrass.TokenExchangeService
- Ensure IAM role has required service permissions

### IoT Core Publishing Pattern
For components that publish to IoT Core:
- Import: `from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2`
- Initialize: `ipc_client = GreengrassCoreIPCClientV2()`
- Publish: `ipc_client.publish_to_iot_core(topic_name=topic, payload=data, qos=1)`
- Use numeric QoS values (0, 1, 2) only

### Configuration Access Pattern
Components can access configuration via environment variables or IPC calls based on recipe configuration.

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

### Debugging Commands

#### Greengrass Nucleus Lite
- Component logs: `journalctl -u ggl.{ComponentName}.service --no-pager -n 10`
- Deployment logs: `journalctl -u ggl.core.ggdeploymentd.service --no-pager -n 10`
- Service status: `systemctl is-active ggl.core.*.service`
- All services: `systemctl status ggl.core.* --no-pager`

