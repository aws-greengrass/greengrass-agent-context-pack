// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Create IoT Core Publisher Component for Greengrass Lite

## What the Agent Should Do
Create a component that publishes messages to AWS IoT Core using Greengrass IPC based on user requirements.

## CRITICAL Requirements
- **NEVER use boto3** for IoT Core publishing - use GreengrassCoreIPCClientV2
- **QoS must be numeric** (0, 1, 2) - NOT string values
- **Access control** under ComponentConfiguration.DefaultConfiguration
- **Use simplified pattern** for containerized environments (no install phase)

## Step 1: Create Component Workspace
Create a timestamped workspace directory and navigate to it. Generate unique names for the component and S3 bucket using timestamps to avoid conflicts.

## Step 2: Create Publisher Script
**AWS Service Access Note**: 
If your component needs S3, DynamoDB, or other AWS services beyond IoT Core, 
see `components/token-exchange-service-guide.md` for TES dependency requirements.

Create a Python script that:
- Uses GreengrassCoreIPCClientV2 for IoT Core communication
- Publishes messages to the specified IoT topic based on user requirements
- Uses numeric QoS values (0, 1, or 2) as specified by user
- Handles graceful shutdown with proper signal handling
- Includes error handling and logging with flush=True
- Implements the publishing pattern requested by user (periodic, one-time, event-driven, etc.)

**Key Implementation Details:**
- Import: `from awsiot.greengrasscoreipc.clientv2 import GreengrassCoreIPCClientV2`
- Method: `ipc_client.publish_to_iot_core(topic_name=topic, payload=payload, qos=numeric_value)`
- Message format and content based on user specifications
- Publishing frequency/trigger based on user requirements

## Step 3: Create Component Recipe
Create a recipe file with:
- Unique component name and version
- Platform specification for aws_nucleus_lite
- Access control for aws.greengrass.ipc.mqttproxy service
- Operation: "aws.greengrass#PublishToIoTCore"
- Resources matching the IoT topics specified by user
- Simplified lifecycle with only run phase
- Artifact URI pointing to S3 location

**Critical Recipe Elements:**
- Access control under ComponentConfiguration.DefaultConfiguration
- Topic resources must match what the component will publish to
- Use simplified lifecycle pattern for containerized environments

## Step 4: Upload and Deploy Component
- Create S3 bucket and upload the script
- Create component version in AWS Greengrass service
- Setup thing group for deployment (required for Greengrass Lite)
- Add device to thing group
- Ensure IAM permissions for S3 access and IoT operations
- Create and execute deployment targeting the thing group

## Step 5: Verify Component Success
Monitor the deployment and component functionality by:
- Checking deployment status
- Viewing component logs
- Confirming expected publishing behavior
- Verifying component service status

## Expected Success Indicators
- Component service shows as active and running
- IPC client initializes successfully
- Messages publish to IoT Core as expected per user requirements
- No permission or QoS errors in logs
- Deployment status shows as active

## Common Failure Patterns and Solutions

### QoS parsing errors
**Cause**: Using string QoS values
**Solution**: Use numeric QoS values (0, 1, 2)

### IPC client initialization failures
**Cause**: Missing or incorrect access control permissions
**Solution**: Verify access control configuration matches component requirements

### S3 access denied during deployment
**Cause**: IAM role lacks required permissions
**Solution**: Add necessary S3 and IoT permissions to Token Exchange Role

### Component runtime failures
**Cause**: Script errors or environment issues
**Solution**: Test component logic and verify container environment setup

This approach creates IoT Core publishing components tailored to specific user requirements while following Greengrass Lite best practices.
