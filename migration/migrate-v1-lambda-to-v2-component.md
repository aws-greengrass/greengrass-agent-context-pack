// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Migrate Greengrass V1 Lambda to V2 Generic Component

## Overview

This guide helps migrate AWS IoT Greengrass V1 Lambda functions to V2 generic components for Greengrass Nucleus Lite devices.

## Prerequisites

- Greengrass V1 group with Lambda functions
- AWS credentials configured
- Target Greengrass V2 device running
- Lambda runtime environment (Python, Java, Node.js, etc.)

## CRITICAL: V1 to V2 Key Differences

**⚠️ IMPORTANT Migration Requirements**:

1. **SDK Change Required**: Must replace Greengrass V1 SDK with V2 SDK (Python/Java/Node.js: IoT Device SDK V2; C/C++: Greengrass Component SDK, see sdk-migration-reference.md)
2. **IPC Authorization**: Subscriptions become IPC authorization policies in recipe

## Migration Workflow

### Step 1: Export V1 Group Configuration

**Setup AWS Credentials**:

Ask user to create a credentials file that can be sourced:
```bash
# Example: ~/credentials.sh
export AWS_ACCESS_KEY_ID="your-access-key"
export AWS_SECRET_ACCESS_KEY="your-secret-key"
export AWS_SESSION_TOKEN="your-session-token"  # if using temporary credentials
export AWS_DEFAULT_REGION="us-west-2"
```

Then source it before running export:
```bash
source ~/credentials.sh
```

**Required IAM Permissions**: `greengrass:GetGroup*`, `greengrass:GetFunctionDefinitionVersion`, `greengrass:GetSubscriptionDefinitionVersion`, `lambda:GetFunction`

**Run Export Script**:

```bash
# Run export script
./migration/export_ggv1.sh <GROUP_ID> ./v1-export <REGION>

# Output files:
# - functions.json: Lambda configurations (env vars)
# - subscriptions.json: Message routing (source, target, topic)
# - group.json: Core device and role information
# - lambda-code/<FUNCTION_NAME>/: Extracted Lambda function code
```

### Step 2: Convert Lambda Code to Component Code

Convert Lambda function code using the SDK migration patterns:

1. **SDK Migration**: Replace V1 SDK calls with V2 IPC
   - **Quick Reference**: See `sdk-migration-reference.md` for SDK operation snippets (publish, subscribe, etc.)
   - **Complete Examples**: See `examples/v1-lambda-migration/<language>/<pattern>/` for full working code:
     - **Languages**: python, java, nodejs, c, cpp
     - **Patterns**: local_communication (device-to-device), cloud_communication (device-to-cloud)
     - **Includes**: V1 Lambda (v1_*.ext), **V2 component (v2_*.ext - USE THIS AS TEMPLATE)**, recipe, and build configs
     - **Example**: For Java cloud communication, use `examples/v1-lambda-migration/java/cloud_communication/v2_controller.java` as your template

2. **Long-Running Process**: Convert Lambda handler to continuous loop with signal handling

Key changes:
- **Use V2 example files (v2_*.ext) as templates** - they show the correct SDK, patterns, and structure
- Replace `greengrasssdk` imports with language-specific V2 SDK
- Convert event-driven handler to continuous process
- Add signal handling for graceful shutdown
- Update publish/subscribe to use IPC operations matching your communication pattern

### Step 3: Create Component Recipe

Map V1 configuration to V2 recipe:

**From functions.json**:
```json
{
  "FunctionArn": "arn:aws:lambda:us-west-2:123456789:function:greengrass_sender:1",
  "Runtime": "python3.9"
}
```

**To recipe.yaml**:
```yaml
RecipeFormatVersion: '2020-01-25'
ComponentName: com.example.greengrass_sender
ComponentVersion: '1.0.0'
ComponentDescription: 'Migrated from V1 Lambda greengrass_sender'
ComponentPublisher: Amazon
ComponentConfiguration:
  DefaultConfiguration:
    accessControl:
      aws.greengrass.ipc.pubsub:
        com.example.greengrass_sender:pubsub:1:
          policyDescription: "Publish sensor data"
          operations:
            - "aws.greengrass#PublishToTopic"
          resources:
            - "data/sensor"
Manifests:
  - Platform:
      os: linux
      runtime: aws_nucleus_lite
    Lifecycle:
      run: |
        python3 -u {artifacts:path}/main.py
```

**IMPORTANT**: 
- Do NOT include `Artifacts` section in recipe for local deployment
- For cloud deployment with S3, use `Unarchive: NONE` for single files (not ZIP)
- See deployment guide for artifact configuration details

**From subscriptions.json**:
```json
{
  "Source": "arn:aws:lambda:...:function:sender:1",
  "Subject": "data/sensor",
  "Target": "arn:aws:lambda:...:function:receiver:1"
}
```

**To IPC Authorization**:
- **Publisher** (Source=sender): Add `PublishToTopic` operation for `data/sensor`
- **Subscriber** (Target=receiver): Add `SubscribeToTopic` operation for `data/sensor`

**CRITICAL**: Extract ALL unique topics from subscriptions.json where the Lambda is Source or Target:
- If Lambda is Source → add topic to PublishToTopic resources
- If Lambda is Target → add topic to SubscribeToTopic resources
- List ALL topics in the resources array (don't miss any)

**Output Structure** (after Steps 2 & 3):
```
v1-export/                      # From Step 1
├── functions.json
├── subscriptions.json
├── group.json
└── lambda-code/
    └── <FUNCTION_NAME>/

v2-components/                  # Generated in Steps 2 & 3
└── <COMPONENT_NAME>/
    ├── recipe.yaml
    └── src/
        └── main.py (or main.js, Main.java, etc.)
```

### Step 4: Deploy Component

Deploy the migrated component to your Greengrass Lite device:

**→ See `../deployment/deploy-components-to-greengrass-lite.md` for complete deployment instructions**

The deployment guide covers:
- Local deployment (greengrass-cli)
- Cloud deployment (AWS CLI)
- Thing group setup
- IAM permissions
- Troubleshooting

## Migration Mapping Reference

### Configuration Mapping

| V1 (functions.json) | V2 | Notes |
|---------------------|-----|-------|
| Environment.Variables | ComponentConfiguration.DefaultConfiguration | Direct mapping |
| Pinned: true | Lifecycle.run (continuous) | Long-running process |

### Code Conversion

| V1 SDK | V2 SDK | Operation |
|--------|--------|-----------|
| greengrasssdk.client('iot-data') | awsiot.greengrasscoreipc.connect() | Initialize |
| iot_client.publish() | ipc_client.publish_to_topic() | Publish |
| lambda_handler(event) | StreamHandler.on_stream_event() | Subscribe |

**For SDK migration resources:**
- `./sdk-migration-reference.md` - Quick reference for SDK operation snippets (publish, subscribe, etc.)
- `../examples/v1-lambda-migration/<language>/<pattern>/` - Complete working examples with V1/V2 code, recipes, and build configs

### Authorization Mapping

| V1 | V2 | Location |
|----|----|----|
| Group subscription | IPC authorization policy | recipe.yaml ComponentConfiguration |
| Source → Subject | PublishToTopic operation | Publisher recipe |
| Target ← Subject | SubscribeToTopic operation | Subscriber recipe |

## Recipe Best Practices

### Artifacts Configuration
- **Local deployment**: Omit `Artifacts` section entirely
- **Cloud deployment (S3)**: 
  - Single file: `Unarchive: NONE`
  - Archive: `Unarchive: ZIP` or `Unarchive: TAR`
  - Never use `Unarchive: ZIP` for single Python/JS files

### IPC Authorization
- **List ALL topics**: Review entire subscriptions.json for the Lambda
- **Match operations to subscriptions.json**:
  - Lambda is **Source** for a topic → `PublishToTopic` operation for that topic
  - Lambda is **Target** for a topic → `SubscribeToTopic` operation for that topic
  - **Example from subscriptions.json**:
    ```json
    [
      {"Source": "arn:...function:SensorLambda", "Subject": "sensors/data", "Target": "arn:...function:ProcessorLambda"},
      {"Source": "arn:...function:ProcessorLambda", "Subject": "alerts/high", "Target": "arn:...function:SensorLambda"}
    ]
    ```
    **ProcessorLambda recipe** needs:
    - `SubscribeToTopic` for `sensors/data` (ProcessorLambda is Target)
    - `PublishToTopic` for `alerts/high` (ProcessorLambda is Source)

- **CRITICAL - Match Actual Usage from subscriptions.json**:
  - Grant ONLY the operations and resources the component actually uses
  - Don't grant both operations to all topics unless subscriptions.json shows both
  
  **WRONG Example** (subscriptions.json shows component only subscribes to topic A, publishes to topic B):
  ```yaml
  # subscriptions.json shows:
  # Component is TARGET for "sensors/temperature" (needs SubscribeToTopic)
  # Component is SOURCE for "alerts/high" (needs PublishToTopic)
  
  # WRONG - grants both operations to both topics:
  aws.greengrass.ipc.pubsub:
    com.example.ProcessorLambda:pubsub:1:
      operations:
        - "aws.greengrass#SubscribeToTopic"
        - "aws.greengrass#PublishToTopic"
      resources:
        - "sensors/temperature"
        - "alerts/high"
  ```
  
  **CORRECT Example** (matches subscriptions.json):
  ```yaml
  aws.greengrass.ipc.pubsub:
    com.example.ProcessorLambda:pubsub:1:
      policyDescription: "Subscribe to sensor data"
      operations:
        - "aws.greengrass#SubscribeToTopic"
      resources:
        - "sensors/temperature"
    com.example.ProcessorLambda:pubsub:2:
      policyDescription: "Publish alerts"
      operations:
        - "aws.greengrass#PublishToTopic"
      resources:
        - "alerts/high"
  ```
  
  **ALSO CORRECT** (if subscriptions.json shows component both subscribes AND publishes to same topics):
  ```yaml
  # If subscriptions.json shows component is both SOURCE and TARGET for same topics
  aws.greengrass.ipc.pubsub:
    com.example.ProcessorLambda:pubsub:1:
      policyDescription: "Subscribe and publish to shared topics"
      operations:
        - "aws.greengrass#SubscribeToTopic"
        - "aws.greengrass#PublishToTopic"
      resources:
        - "sensors/temperature"
        - "sensors/humidity"
  ```

- **Wildcard topics**: Use `*` carefully (e.g., `sensors/*` for `sensors/temp`, `sensors/humidity`)

### Lifecycle Script
- **Use -u flag**: `python3 -u` for unbuffered output (proper logging)
- **Correct path**: `{artifacts:path}/main.py` (or appropriate entry point)
- **Executable permissions**: Ensure script is executable for compiled languages

## Verification

Check component logs:
```bash
# Greengrass Lite
journalctl -u ggl.com.example.sensor.service -f

# Expected output
Component started
Published to data/sensor: {'value': 42}
```

## Troubleshooting

- **SDK Import Error**: Install language-specific V2 SDK (see sdk-migration-reference.md for package names)
- **Permission Denied**: Check IPC authorization in recipe matches actual topics used in code
- **Component Not Starting**: Check component logs and verify lifecycle script path is correct
- **No Messages**: Confirm both publisher and subscriber have correct IPC policies

## Working Example

See complete sender/receiver migration example:
- `../examples/v1-lambda-migration/`

### Build Instructions by Language

- **C**: See `../examples/v1-lambda-migration/c/README.md` for build dependencies, CMakeLists.txt setup, and compilation steps
- **C++**: See `../examples/v1-lambda-migration/cpp/README.md` for build configuration
- **Java**: See `pom.xml` in Java examples for Maven dependencies
- **Node.js**: See `package.json` in Node.js examples for npm dependencies
- **Python**: Install SDK with `pip3 install awsiotsdk`
