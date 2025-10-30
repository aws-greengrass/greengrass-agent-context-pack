// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# AI Agent Instructions: Deploy Components to Greengrass Lite Device

## Prerequisites
- Greengrass Lite device already running (use `setup-greengrass-lite-container.md`)
- AWS credentials available
- Container name from setup (e.g., `ggl-{timestamp}`)

## CRITICAL: Greengrass Lite Deployment Requirements

**⚠️ IMPORTANT**: Greengrass Lite has specific deployment requirements that differ from regular Greengrass:

1. **Thing Groups Required**: Deployments MUST target thing groups, not individual things
2. **IAM Permissions**: Token Exchange Role needs S3 read permissions for component artifacts
3. **Case Sensitivity**: Recipe lifecycle keys are case-sensitive (use lowercase `run`)

## CRITICAL: IoT Core Publishing Requirements

**⚠️ FOR IOT CORE PUBLISHING COMPONENTS**:

1. **NEVER use boto3** - Use `GreengrassCoreIPCClientV2` from awsiotsdk
2. **QoS must be numeric** (0, 1, 2) - NOT strings like "AT_LEAST_ONCE"
3. **Access control** must be under `ComponentConfiguration.DefaultConfiguration.accessControl`
4. **Install awsiotsdk** with `--break-system-packages` flag in install phase
5. **Set writable HOME** directory: `export HOME=/tmp/gg-home`
6. **Use aws.greengrass.ipc.mqttproxy** for access control service
7. **Operation**: `aws.greengrass#PublishToIoTCore`

## Component Development Workflow

### Step 1: Create Component Workspace
```bash
COMP_TIMESTAMP=$(date +%s)
COMPONENT_NAME="com.example.YourComponent$COMP_TIMESTAMP"  # Use descriptive name
mkdir -p /tmp/component-$COMP_TIMESTAMP
cd /tmp/component-$COMP_TIMESTAMP
```

### Step 2: Create Component Files

Create your component script with these requirements:
- Use `flush=True` in Python print statements for proper logging
- Include proper error handling and graceful shutdown
- Use timestamp in component name for uniqueness

**Example Python Template**:
```python
#!/usr/bin/env python3
import time
import sys
import signal

def signal_handler(sig, frame):
    print("Component shutting down gracefully...", flush=True)
    sys.exit(0)

def main():
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    print(f"Component {COMP_TIMESTAMP} starting...", flush=True)

    # Your component logic here
    while True:
        # Component work (e.g., publish to topic, process data, etc.)
        print(f"Component working at {time.strftime('%H:%M:%S')}", flush=True)
        time.sleep(30)  # Adjust interval as needed

if __name__ == "__main__":
    main()
```

## Cloud Deployment (Recommended)

Cloud deployment provides the complete AWS Greengrass experience, allowing customers to use the full AWS IoT Greengrass service including the AWS Console, deployment management, and fleet operations.

### Step 1: Create S3 Bucket and Upload Artifact
```bash
BUCKET_NAME="greengrass-components-$COMP_TIMESTAMP"
aws s3 mb s3://$BUCKET_NAME --region us-east-1
aws s3 cp your_component_script.py s3://$BUCKET_NAME/$COMPONENT_NAME/1.0.0/your_component_script.py --region us-east-1
```

### Step 2: Create Component Recipe for Cloud
Create `recipe.yaml`:
```yaml
RecipeFormatVersion: 2020-01-25
ComponentName: {COMPONENT_NAME}
ComponentVersion: 1.0.0
ComponentDescription: {COMPONENT_DESCRIPTION}
ComponentPublisher: {PUBLISHER_NAME}
Manifests:
  - Platform:
      os: linux
      runtime: aws_nucleus_lite
    Lifecycle:
      run: |
        python3 {artifacts:path}/your_component_script.py
    Artifacts:
      - Uri: s3://{BUCKET_NAME}/{COMPONENT_NAME}/1.0.0/your_component_script.py
```

**CRITICAL**:
- Use lowercase `run` in Lifecycle (Greengrass Lite is case-sensitive)
- Replace placeholders: `{COMPONENT_NAME}`, `{COMPONENT_DESCRIPTION}`, `{PUBLISHER_NAME}`, `{BUCKET_NAME}`
- Adjust script name and path as needed

### Step 3: Create Component Version in AWS
```bash
aws greengrassv2 create-component-version --inline-recipe fileb://recipe.yaml --region us-east-1
```

### Step 4: Setup Thing Group and IAM Permissions

**CRITICAL STEP**: Greengrass Lite requires thing group deployments and proper IAM permissions.

#### Create Thing Group
```bash
THING_GROUP_NAME="GreengrassLiteGroup-$COMP_TIMESTAMP"
THING_NAME="GreengrassLiteCore-{your_setup_timestamp}"  # Use your thing name from setup

# Create thing group
aws iot create-thing-group --thing-group-name $THING_GROUP_NAME --region us-east-1

# Add thing to group
aws iot add-thing-to-thing-group --thing-group-name $THING_GROUP_NAME --thing-name $THING_NAME --region us-east-1
```

#### Add Required IAM Permissions
```bash
IAM_ROLE_NAME="GreengrassV2TokenExchangeRole-{your_setup_timestamp}"  # Use your role name from setup

# Create S3 access policy
cat > s3-policy.json << EOF
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": [
        "s3:GetObject"
      ],
      "Resource": "arn:aws:s3:::$BUCKET_NAME/*"
    },
    {
      "Effect": "Allow",
      "Action": [
        "logs:CreateLogGroup",
        "logs:CreateLogStream",
        "logs:PutLogEvents",
        "logs:DescribeLogStreams"
      ],
      "Resource": "arn:aws:logs:*:*:*"
    }
  ]
}
EOF

# Add additional permissions based on component needs (e.g., IoT Core, S3, etc.)
# For IoT publishing components, add:
# "iot:Publish", "iot:Subscribe", "iot:Connect", "iot:Receive"

# Create and attach policy
aws iam create-policy --policy-name "GreengrassComponentAccess-$COMP_TIMESTAMP" --policy-document file://s3-policy.json --region us-east-1
aws iam attach-role-policy --role-name $IAM_ROLE_NAME --policy-arn "arn:aws:iam::$(aws sts get-caller-identity --query Account --output text):policy/GreengrassComponentAccess-$COMP_TIMESTAMP" --region us-east-1

# Also attach S3 read-only access
aws iam attach-role-policy --role-name $IAM_ROLE_NAME --policy-arn "arn:aws:iam::aws:policy/AmazonS3ReadOnlyAccess" --region us-east-1
```

### Step 5: Create Cloud Deployment to Thing Group
```bash
cat > deployment.json << EOF
{
  "targetArn": "arn:aws:iot:us-east-1:$(aws sts get-caller-identity --query Account --output text):thinggroup/$THING_GROUP_NAME",
  "deploymentName": "{DeploymentName}-$COMP_TIMESTAMP",
  "components": {
    "$COMPONENT_NAME": {
      "componentVersion": "1.0.0"
    }
  }
}
EOF

aws greengrassv2 create-deployment --cli-input-json file://deployment.json --region us-east-1
```

### Step 6: Monitor Deployment
```bash
# Check deployment status
aws greengrassv2 list-deployments --target-arn "arn:aws:iot:us-east-1:$(aws sts get-caller-identity --query Account --output text):thinggroup/$THING_GROUP_NAME" --region us-east-1

# Monitor device logs for deployment processing
CONTAINER_NAME="ggl-{your_setup_timestamp}"  # Use your container name
podman exec $CONTAINER_NAME journalctl -u ggl.core.ggdeploymentd.service -f
```

**Expected Log Sequence**:
1. "Got message from IoT Core" - Job received
2. "HTTP code: 200" - S3 artifact downloaded successfully
3. "Component succeeded" - Component deployed
4. "Completed deployment processing and reporting job as SUCCEEDED" - Deployment complete

### Step 7: Verify Component Deployment
```bash
# Check component service status
podman exec $CONTAINER_NAME systemctl status ggl.$COMPONENT_NAME.service --no-pager

# View component logs
podman exec $CONTAINER_NAME journalctl -u ggl.$COMPONENT_NAME.service --no-pager -f
```

## Local Deployment (Development/Testing Only)

Use local deployment only for rapid development iteration or when cloud connectivity is limited. For production and customer demonstrations, prefer cloud deployment.

### Step 1: Create Local Component Recipe
Create `recipe-local.yaml`:
```yaml
RecipeFormatVersion: 2020-01-25
ComponentName: {COMPONENT_NAME}
ComponentVersion: 1.0.0
ComponentDescription: {COMPONENT_DESCRIPTION}
ComponentPublisher: {PUBLISHER_NAME}
Manifests:
  - Platform:
      os: linux
      runtime: aws_nucleus_lite
    Lifecycle:
      run: |
        python3 {artifacts:path}/your_component_script.py
    Artifacts:
      - Uri: file:///var/lib/greengrass/packages/artifacts/{COMPONENT_NAME}/1.0.0/your_component_script.py
```

### Step 2: Prepare Component Store
```bash
CONTAINER_NAME="ggl-{your_setup_timestamp}"  # Use your container name

# Create directory structure in container
podman exec $CONTAINER_NAME mkdir -p /var/lib/greengrass/packages/recipes
podman exec $CONTAINER_NAME mkdir -p /var/lib/greengrass/packages/artifacts/$COMPONENT_NAME/1.0.0
```

### Step 3: Copy Files to Container
```bash
# Copy recipe with proper naming convention
podman cp recipe-local.yaml $CONTAINER_NAME:/var/lib/greengrass/packages/recipes/$COMPONENT_NAME-1.0.0.yaml

# Copy artifact to proper location
podman cp your_component_script.py $CONTAINER_NAME:/var/lib/greengrass/packages/artifacts/$COMPONENT_NAME/1.0.0/

# Set correct ownership
podman exec $CONTAINER_NAME chown -R ggcore:ggcore /var/lib/greengrass/packages/
```

### Step 4: Deploy Component Locally
```bash
podman exec $CONTAINER_NAME /usr/local/bin/ggl-cli deploy --add-component $COMPONENT_NAME=1.0.0
```

### Step 5: Verify Local Deployment
```bash
# Check service status
podman exec $CONTAINER_NAME systemctl status ggl.$COMPONENT_NAME.service --no-pager

# View component logs
podman exec $CONTAINER_NAME journalctl -u ggl.$COMPONENT_NAME.service --no-pager -f
```

## Component Management

### Update Component
```bash
# Stop current version
podman exec $CONTAINER_NAME systemctl stop ggl.$COMPONENT_NAME.service

# Update files (replace with your updated script)
podman cp updated_script.py $CONTAINER_NAME:/var/lib/greengrass/packages/artifacts/$COMPONENT_NAME/1.0.0/your_component_script.py

# Restart
podman exec $CONTAINER_NAME systemctl start ggl.$COMPONENT_NAME.service
```

### Remove Component
```bash
# Stop and disable service
podman exec $CONTAINER_NAME systemctl stop ggl.$COMPONENT_NAME.service
podman exec $CONTAINER_NAME systemctl disable ggl.$COMPONENT_NAME.service

# Remove files
podman exec $CONTAINER_NAME rm -rf /var/lib/greengrass/packages/artifacts/$COMPONENT_NAME
podman exec $CONTAINER_NAME rm /var/lib/greengrass/packages/recipes/$COMPONENT_NAME-1.0.0.yaml

# Reload systemd
podman exec $CONTAINER_NAME systemctl daemon-reload
```

## Troubleshooting

### Recovery from Deployment Failures
When deployments fail or components block new deployments:
- Clean slate approach: stop services, remove /var/lib/greengrass/*, restart
- Check deployment logs first: `journalctl -u ggl.core.ggdeploymentd.service`

### Common Issues and Solutions

#### 1. Deployment Fails with "No deployment to process"
**Cause**: Deploying to individual thing instead of thing group
**Solution**: Create thing group and deploy to group ARN

#### 2. HTTP 403 Errors During Artifact Download
**Cause**: Token Exchange Role lacks S3 permissions
**Solution**: Add S3 read permissions to IAM role

#### 3. Service Fails to Start
**Cause**: Recipe casing issues (`Run` vs `run`)
**Solution**: Use lowercase lifecycle keys in recipe

#### 4. No Output in Logs
**Cause**: Python output not flushed
**Solution**: Add `flush=True` to print statements

#### 5. Component Permissions Issues
**Cause**: Missing IAM permissions for component functionality
**Solution**: Add required permissions to Token Exchange Role (IoT, S3, etc.)

### Debug Commands
```bash
# Check deployment logs
podman exec $CONTAINER_NAME journalctl -u ggl.core.ggdeploymentd.service --no-pager -n 50

# Check component status
podman exec $CONTAINER_NAME systemctl status ggl.$COMPONENT_NAME.service

# Check file permissions
podman exec $CONTAINER_NAME ls -la /var/lib/greengrass/packages/artifacts/$COMPONENT_NAME/

# View systemd unit file
podman exec $CONTAINER_NAME cat /etc/systemd/system/ggl.$COMPONENT_NAME.service

# Check Greengrass core services
podman exec $CONTAINER_NAME systemctl status greengrass-lite.target
```

## Component Development Guidelines

### For IoT Publishing Components
- Add IoT permissions: `"iot:Publish"`, `"iot:Subscribe"`, `"iot:Connect"`, `"iot:Receive"`
- Use Greengrass IPC SDK for IoT Core communication
- Handle connection retries and error cases

### For Data Processing Components
- Add necessary AWS service permissions (S3, DynamoDB, etc.)
- Implement proper error handling and logging
- Consider memory and CPU constraints

### For Sensor Components
- Handle hardware access permissions
- Implement graceful shutdown for cleanup
- Add appropriate retry logic for sensor failures

## Key Differences from Regular Greengrass
- **Thing Groups Required**: Must deploy to thing groups, not individual things
- **IAM Permissions**: Token Exchange Role needs explicit permissions for component functionality
- **Case Sensitivity**: Lifecycle keys must be lowercase
- **Runtime Specification**: Use `runtime: aws_nucleus_lite` in recipe Platform section
- **Cloud Deployment**: Provides full AWS Greengrass service experience
- **Local Deployment**: Available via `ggl-cli` for development iteration
- **Component Artifacts**: Must be in exact expected directory structure
- **Python Scripts**: Need explicit output flushing for systemd logs

## Benefits of Cloud Deployment
- **Full AWS Experience**: Customers experience the complete AWS IoT Greengrass service
- **AWS Console Integration**: Manage deployments through AWS Console
- **Fleet Management**: Scale deployments across multiple devices
- **Deployment History**: Track deployment status and history
- **Component Versioning**: Manage component versions in the cloud
- **Rollback Capabilities**: Easy rollback to previous component versions

## Success Message After Component Deployment
**MANDATORY: After successful component deployment, provide this exact message to the user:**

---

## ✅ Component Successfully Deployed!

Your component is now running on the Greengrass Lite device. Here's what I can help you with next:

**Component Management:**
- **Monitor component status** - Check logs and health status of deployed components
- **Update component configuration** - Modify component settings without redeployment
- **Deploy additional components** - Add more functionality to your device
- **Create component dependencies** - Build components that work together

**Development Workflow:**
- **Iterate on component code** - Update and redeploy components for development
- **Test different component patterns** - Try publishers, subscribers, or processing components
- **Debug component issues** - Analyze logs and troubleshoot problems
- **Scale to multiple devices** - Deploy to thing groups for fleet management

**Advanced Features:**
- **Inter-component communication** - Enable local pub-sub messaging between components
- **Component lifecycle hooks** - Add startup, shutdown, and restart logic
- **Configuration management** - Dynamic configuration updates
- **Performance optimization** - Monitor and optimize component resource usage

**Quick Next Steps:**
- Say **"show component logs"** to monitor your deployed component
- Say **"create another component"** to add more functionality
- Say **"update component configuration"** to modify settings
- Say **"deploy to more devices"** for fleet management

What would you like to work on next?

---
