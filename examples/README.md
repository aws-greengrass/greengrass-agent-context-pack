# Greengrass Component Examples

This directory contains complete, working examples of Greengrass components that AI agents can use as templates for common IoT scenarios.

## Available Examples

### Basic Components
- `hello-world/` - Simple component that logs messages (Python)
- `sensor-simulator/` - Simulates sensor data with configurable intervals
- `file-watcher/` - Monitors directory for file changes

### AWS Service Integration
- `iot-core-publisher/` - Publishes messages to AWS IoT Core
- `s3-uploader/` - Uploads files to S3 buckets
- `dynamodb-writer/` - Writes data to DynamoDB tables

### Inter-Component Communication
- `ipc-publisher/` - Publishes messages via Greengrass IPC
- `ipc-subscriber/` - Subscribes to IPC messages
- `pubsub-bridge/` - Bridges local IPC to IoT Core

### Advanced Patterns
- `lambda-component/` - Lambda function as Greengrass component
- `docker-component/` - Containerized component example
- `multi-language/` - Components in different languages communicating

## Usage for AI Agents

1. **Copy the entire example directory** to user's workspace
2. **Modify the component name** and configuration as needed
3. **Update AWS resource references** (topics, buckets, tables)
4. **Test locally** before deployment
5. **Follow deployment patterns** from hands-on labs

## Template Structure

Each example includes:
- `recipe.json` - Component recipe with proper configuration
- `src/` - Source code with error handling
- `README.md` - Specific setup and usage instructions
- `test/` - Basic testing approach (where applicable)

## Important Notes

- All examples include proper error handling and logging
- AWS credentials are handled via Token Exchange Service (TES)
- Components follow Greengrass best practices
- Examples are designed for both Greengrass and Lite (where compatible)
