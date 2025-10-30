// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Setup Greengrass Lite Container

## What the Agent Should Do
Set up AWS IoT Greengrass Lite running in a container with all necessary AWS resources and configuration.

## Step 1: Prepare Workspace
Create a temporary directory with timestamp and navigate to it. Clone the official Greengrass Lite repository from GitHub: `https://github.com/aws-greengrass/aws-greengrass-lite.git`

## Step 2: Create AWS IoT Resources
Use AWS CLI to create:
- An IoT Thing with a unique name (e.g., "GreengrassLiteCore-{timestamp}" or "GreengrassLiteCore-{random}") in us-east-1 region
- Device certificates (active certificate with private key)
- Get the IoT Data endpoint and Credential Provider endpoint for us-east-1

Save the certificate ARN, certificate PEM, and private key from the certificate creation response.

## Step 2.5: Create IAM Role and Role Alias for Token Exchange Service
Create the required IAM role and role alias for the Token Exchange Service (TES):
- Create IAM role named "GreengrassV2TokenExchangeRole-{timestamp}" with trust policy from `container-configs/iam-trust-policy.json` (allows "credentials.iot.amazonaws.com" service to assume the role) AND attach AmazonS3ReadOnlyAccess policy for artifact downloads
- Create IoT role alias named "GreengrassV2TokenExchangeRoleAlias-{timestamp}" pointing to the IAM role ARN
- If role/alias already exists, continue with existing ones
- Save the role alias name for use in Step 4 configuration

## Step 3: Create IoT Policy and Attachments
Create an IoT policy with a unique name (e.g., "GreengrassLitePolicy-{timestamp}" or "GreengrassLitePolicy-{random}") that allows all IoT and Greengrass actions on all resources. Attach this policy to the certificate and attach the certificate to the IoT Thing.

## Step 4: Prepare Configuration Files
Create a certificates directory at `run/certs/` and save:
- Certificate PEM as `device.pem`
- Private key as `device.key`
- Download Amazon Root CA certificate as `AmazonRootCA1.pem`

Create `run/config.yaml` with system paths pointing to `/var/lib/greengrass/certs/` for the certificates, the unique thing name created in Step 2, the unique IoT role alias from Step 2.5, and services configuration for NucleusLite with the IoT endpoints obtained earlier. Refer to the configuration provided in the file `container-configs/lite-config.yaml`.

## Step 5: Build Container
Create a Dockerfile that:
- Uses Ubuntu 24.04 base image
- Installs systemd, build tools, pkg-config, git, python3-pip, and required libraries (SSL, curl, SQLite, YAML, systemd, URI parser, UUID, libevent, libzip)
- Creates gg_component and ggcore users and groups
- Copies the Greengrass Lite source and builds it with cmake and make install
- Enables the greengrass-lite systemd target
- Runs systemd as the main process

**Enhanced Dockerfile Template:**
```dockerfile
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    systemd \
    build-essential \
    cmake \
    git \
    python3-pip \
    pkg-config \
    libssl-dev \
    libcurl4-openssl-dev \
    libsqlite3-dev \
    libyaml-dev \
    libsystemd-dev \
    liburiparser-dev \
    uuid-dev \
    libevent-dev \
    libzip-dev \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd -r ggcore && useradd -r -g ggcore ggcore
RUN groupadd -r gg_component && useradd -r -g gg_component gg_component

COPY aws-greengrass-lite /opt/aws-greengrass-lite
WORKDIR /opt/aws-greengrass-lite

RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install

RUN systemctl enable greengrass-lite.target

CMD ["/usr/lib/systemd/systemd"]
```

Build the container image with Podman using a unique tag (e.g., "ggl:{timestamp}" or "ggl:{random}"). Use Podman instead of Docker for better security and rootless container support.

## Step 6: Run and Configure Container
Start the container with a unique name (e.g., "ggl-{timestamp}" or "ggl-{random}") using Podman in privileged mode with systemd. Then:
- Create `/run/greengrass` directory owned by ggcore user
- Copy the config.yaml to `/etc/greengrass/`
- Copy the certificates directory to `/var/lib/greengrass/`
- Set proper ownership (ggcore:ggcore) on certificate files

## CRITICAL Container Requirements

### /run/greengrass Ownership
**ALWAYS ensure /run/greengrass is owned by ggcore:ggcore**
- Use tmpfs mount for better permission control: `--tmpfs /run/greengrass:rw,size=100m`
- After container start: `podman exec $CONTAINER_NAME chown ggcore:ggcore /run/greengrass`
- Socket permission failures indicate ownership issues

### All 8 Core Services Required
For devices to appear in Greengrass console, ALL services must be active:
- ggl.core.ggconfigd.service (config daemon) - **CRITICAL for IPC**
- ggl.core.iotcored.service (IoT Core proxy) - **CRITICAL for connectivity**
- ggl.core.tesd.service (credential provider) - **CRITICAL for authentication**
- ggl.core.ggdeploymentd.service (deployment processor)
- ggl.core.gg-fleet-statusd.service (fleet status) - **CRITICAL for console visibility**
- ggl.core.ggpubsubd.service (pub-sub daemon)
- ggl.core.gghealthd.service (health monitoring)
- ggl.core.ggipcd.service (IPC proxy) - **CRITICAL for component communication**

**Post-Setup Component Preparation:**
After the container is running, install Python dependencies for IoT components:
```bash
podman exec $CONTAINER_NAME mkdir -p /tmp/gg-home
podman exec $CONTAINER_NAME bash -c "export HOME=/tmp/gg-home && python3 -m pip install awsiotsdk --break-system-packages"
```

Note: Use Podman instead of Docker for improved security, rootless operation, and better integration with systemd.

## Step 7: Start Services
Enable these systemd services in the container:
- ggl.core.ggconfigd.service (config daemon)
- ggl.core.iotcored.service (IoT Core proxy)
- ggl.core.tesd.service (credential provider)
- ggl.core.ggdeploymentd.service (deployment processor)
- ggl.core.gg-fleet-statusd.service (fleet status)
- ggl.core.ggpubsubd.service (pub-sub daemon)
- ggl.core.gghealthd.service (health monitoring)
- ggl.core.ggipcd.service (IPC proxy)

Start the greengrass-lite.target and wait 10 seconds for initialization.

## Step 8: Verify Success
Check that all ggl services are active and running. The setup is complete when all 8 core services show as "loaded active running".

## Step 9: Success Message and Next Steps
**MANDATORY: After successful setup, provide this exact message to the user:**

---

## ✅ Greengrass Lite Container Setup Complete!

Your Greengrass Lite environment is now running and ready for development. Here's what I can help you with next:

**Component Development:**
- **Create custom IoT components** - I can generate component recipes and code templates for Python, shell scripts, or other languages
- **Deploy components to your device** - I can create deployments targeting your Greengrass Lite device
- **Build IoT Core publisher components** - Components that send sensor data or telemetry to AWS IoT Core
- **Create subscriber components** - Components that react to IoT Core messages or local pub-sub topics

**Advanced Capabilities:**
- **Component lifecycle management** - Start, stop, restart, and monitor component health
- **Configuration management** - Update component configurations dynamically
- **Local pub-sub messaging** - Enable inter-component communication
- **Troubleshooting and debugging** - Analyze logs, diagnose issues, and optimize performance

**Quick Start Options:**
- Say **"create a simple publisher component"** for a basic IoT Core publisher
- Say **"show me component templates"** to see available patterns
- Say **"deploy a component"** to start the deployment process
- Say **"help with troubleshooting"** if you encounter any issues

What would you like to work on next?

---

## Expected Result
A running Podman container with unique name containing Greengrass Lite fully operational, connected to AWS IoT Core, and ready for component deployments.

## Cleanup Information
Provide commands to stop/remove the Podman container and clean up the temporary directory. Optionally mention how to remove the AWS IoT resources (detach policies, delete certificates, delete thing) and IAM resources (delete role alias, delete IAM role).

## Key Variables the Agent Should Track
- TEMP_DIR: Temporary working directory path
- THING_NAME: Unique thing name with timestamp/random suffix
- POLICY_NAME: Unique policy name with timestamp/random suffix
- ROLE_ALIAS_NAME: Unique role alias name with timestamp/random suffix
- IAM_ROLE_NAME: Unique IAM role name with timestamp/random suffix
- IAM_ROLE_ARN: ARN of the created IAM role (for role alias creation)
- REGION: "us-east-1"
- Container name: Unique container name with timestamp/random suffix
- Container image tag: Unique tag with timestamp/random suffix
- Certificate ARN, IoT endpoints (for configuration)
- Container runtime: Podman (preferred over Docker for containerized environments)

**Important**: Use timestamps or random strings to ensure all resource names are unique and avoid conflicts with existing AWS resources.
