// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Setup Greengrass in Container

## What the Agent Should Do
Set up AWS IoT Greengrass (Nucleus) in a Podman container for component development and deployment.

## CRITICAL Requirements - Container Setup
- **MUST run container as ROOT** - Greengrass (full runtime) requires root privileges for component execution
- **MUST use --init flag** - Required for proper process management and shutdown
- **MUST use --privileged** - Required for system-level operations
- **Ubuntu base recommended** - Better Python/system compatibility than Amazon Linux

## Step 1: Create Ubuntu-Based Dockerfile
Create Dockerfile with these essential elements:
- Use Ubuntu 22.04 as base image
- **Set timezone non-interactively** (DEBIAN_FRONTEND=noninteractive, TZ=<timezone>) to prevent build hangs
- Install: openjdk-11-jdk, python3, python3-pip, curl, unzip, sudo, awscli, tzdata
- Create ggc_user and ggc_group with sudo NOPASSWD access
- Create /greengrass/v2 directory owned by ggc_user:ggc_group
- Download latest Greengrass Nucleus from cloudfront endpoint (use /lib/Greengrass.jar path)
- **CRITICAL**: Container MUST run as root throughout - Do NOT switch to ggc_user at any point
- **CRITICAL**: All Greengrass processes must run with root privileges for proper system access
- Set entrypoint to custom script

## Step 2: Create Entrypoint Script
Create entrypoint script that:
- Checks for required THING_NAME environment variable
- Gets AWS IoT endpoint using AWS CLI
- Installs Greengrass Nucleus if not present using java command (NOT sudo)
- Configures with standard parameters: thing group, TES role, component user as ggc_user:ggc_group
- **CRITICAL**: Do NOT include --region parameter (deprecated in newer versions)
- Starts existing installation if already present

## Step 3: Build and Run Container
**CRITICAL Container Flags Required:**
- `--init` - Required for proper process management
- `--privileged` - Required for system operations
- Prefer podman to build and run the container unless the user specifies otherwise. Give options to the user if it does not work
- **Container runs as root** - Required for Greengrass system-level operations
- Mount AWS credentials to `/root/.aws:ro` (since container runs as root)
- Set environment variables: THING_NAME, AWS_DEFAULT_REGION

## Step 4: Install Python Dependencies
After container starts, install awsiotsdk package using pip3 in the running container.

## Step 5: Verify Setup
Check that:
- Greengrass processes are running
- Greengrass logs show successful startup
- Container can execute simple test components

## Component Development Notes

### AWS Service Integration
Components accessing AWS services require Token Exchange Service (TES) dependency.
See: `components/token-exchange-service-guide.md`

### Recipe Format
Use standard Greengrass v2 recipe format with:
- Access control for aws.greengrass.ipc.mqttproxy
- Operations: "aws.greengrass#PublishToIoTCore"
- Resources: specify topic names
- Script execution using python3 -c with inline code

### Python IPC Client
Import GreengrassCoreIPCClientV2 from awsiot.greengrasscoreipc.clientv2
Use publish_to_iot_core method with topic_name, payload, and qos parameters

## Critical Differences from Greengrass Lite

| Aspect | Greengrass | Greengrass Lite |
|--------|----------------|-----------------|
| Container User | Root required | ggcore user |
| Container Flags | --init --privileged | --privileged only |
| Python Dependencies | Manual pip install | Built-in |
| Credentials Path | /root/.aws | /home/ggc_user/.aws |

## Common Issues and Solutions

### Components fail with "sudo: a password is required"
**Root Cause**: Container not running as root or missing --init flag
**Solution**: Ensure Dockerfile stays as root user and container runs with --init --privileged

### "No module named 'awsiot'" error
**Root Cause**: Python IPC client not installed in container
**Solution**: Execute pip3 install awsiotsdk in running container

### Components don't redeploy after changes
**Root Cause**: Run-once components need version increment to trigger re-execution
**Solution**: Create new component version and deploy updated version

### Greengrass fails to start in container
**Root Cause**: Missing required container flags or incorrect credential mounting
**Solution**: Verify --init flag usage and credentials mounted to /root/.aws

### Container build hangs during timezone configuration
**Root Cause**: Interactive timezone selection prompt
**Solution**: Set DEBIAN_FRONTEND=noninteractive and TZ environment variables in Dockerfile

### "--region" parameter error during provisioning
**Root Cause**: Using deprecated --region parameter in newer Greengrass versions
**Solution**: Remove --region parameter from installation command

### Greengrass download/extraction fails
**Root Cause**: Greengrass zip structure changed, incorrect extraction path
**Solution**: Use correct path structure (/lib/Greengrass.jar instead of /GreengrassInstaller/lib/)

### Device shows offline despite successful startup
**Root Cause**: Provisioning failed but Greengrass runs in offline mode
**Solution**: Check AWS credentials, verify IoT endpoint retrieval, re-run provisioning

## Success Indicators
- Container runs as root without switching users
- Greengrass Nucleus starts and connects to AWS IoT
- Components execute without sudo permission errors
- Component logs show exitCode=0 and successful execution
- IPC client successfully publishes to IoT Core topics

## Cloud Component Deployment Workflow

### Prerequisites
- Container running with AWS connectivity established
- Python dependencies installed: `pip3 install awsiotsdk` in running container

### Deployment Process
1. Create component version in AWS Greengrass service with S3 artifacts
2. Deploy to thing group (preferred) or individual device
3. Monitor deployment status in `/greengrass/v2/logs/greengrass.log`
4. Verify component execution in component-specific logs

### Deployment Success Indicators
- Component lifecycle: NEW → INSTALLED → STARTING → RUNNING
- IPC connection established in logs
- Component publishes messages successfully
- No module import errors for awsiot libraries

## Success Message and Next Steps
**MANDATORY: After successful setup, provide this exact message to the user:**

---

## ✅ Greengrass Container Setup Complete!

Your Greengrass environment is now running and ready for development. Here's what I can help you with next:

**Component Development:**
- **Create custom IoT components** - I can generate component recipes and code templates for Python, Java, shell scripts, or other languages
- **Deploy components to your device** - I can create deployments targeting your Greengrass device
- **Build IoT Core publisher components** - Components that send sensor data or telemetry to AWS IoT Core
- **Create subscriber components** - Components that react to IoT Core messages or local pub-sub topics

**Advanced Capabilities:**
- **Component lifecycle management** - Start, stop, restart, and monitor component health
- **Configuration management** - Update component configurations dynamically
- **Local pub-sub messaging** - Enable inter-component communication
- **Docker component deployment** - Deploy containerized components
- **Troubleshooting and debugging** - Analyze logs, diagnose issues, and optimize performance

**Quick Start Options:**
- Say **"create a simple publisher component"** for a basic IoT Core publisher
- Say **"show me component templates"** to see available patterns
- Say **"deploy a component"** to start the deployment process
- Say **"help with troubleshooting"** if you encounter any issues

What would you like to work on next?

---
