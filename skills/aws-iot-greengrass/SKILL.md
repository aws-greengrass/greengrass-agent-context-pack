---
name: aws-iot-greengrass
description: Guides AI agents through setting up AWS IoT Greengrass (Nucleus and Lite) in containerized environments, creating and deploying custom IoT components, migrating V1 Lambda functions to V2 components, and troubleshooting common issues. Covers both the full Java-based Nucleus runtime and the lightweight C-based Nucleus Lite runtime for constrained devices.
license: CC-BY-SA-4.0
metadata:
  author: aws-greengrass
  version: "1.0"
---

## Overview

AWS IoT Greengrass is an edge runtime for IoT devices. This skill covers experimentation and quick start scenarios in containerized environments — **not production deployments**.

Two runtimes are available:
- **Greengrass Nucleus** — Full Java runtime with complete component lifecycle management and IPC support
- **Greengrass Nucleus Lite** — Lightweight C runtime for constrained devices (<5MB RAM), optimized for minimal footprint

When a user doesn't specify which runtime, always ask before proceeding.

## When to Use This Skill

Use this skill when:
- Setting up Greengrass Nucleus or Lite in containers for experimentation
- Creating or deploying custom IoT components
- Migrating Greengrass V1 Lambda functions to V2 components
- Troubleshooting Greengrass container or component issues
- Working with Greengrass IPC, MQTT, or component recipes

Do NOT use this skill for:
- Production device provisioning or fleet management
- AWS IoT Core without Greengrass (use IoT Core docs directly)
- Other edge runtimes (AWS IoT SiteWise, FreeRTOS, etc.)

## Workflow Selection

Choose the right starting point:

### Setup
- **Full-featured development/testing** → [Setup Nucleus](./references/setup/setup-greengrass-container.md)
- **Constrained devices / lightweight** → [Setup Lite](./references/setup/setup-greengrass-lite-container.md)

### Component Development
- **Create components, recipes, IPC patterns** → [Component Development](./references/components/component-development.md)
- **Deploy to a running device** → [Deployment](./references/deployment/deploy-components-to-greengrass-lite.md)

### Migration
- **Migrate V1 Lambda functions to V2** → [Migration V1 to V2](./references/migration/migrate-v1-lambda-to-v2-component.md)

### Troubleshooting
- **Container, service, or component issues** → [Troubleshooting](./references/troubleshooting/common-issues.md)

## Common Mistakes

Avoid these frequent errors when working with Greengrass:

1. **Using `Run` instead of `run` in Lite recipes**: Greengrass Lite is case-sensitive for all recipe lifecycle keys. Use lowercase: `run`, `install`, `startup`, `shutdown`.

2. **Deploying to individual things instead of thing groups**: Greengrass Lite requires deployments to target thing groups, not individual things. Always create a thing group and deploy to the group ARN.

3. **Using boto3 for IoT Core publishing**: Never use boto3 to publish to IoT Core from Greengrass components. Use `GreengrassCoreIPCClientV2` from the `awsiotsdk` package instead.

4. **Using string QoS values**: QoS must be numeric (0, 1, 2), not strings like `"AT_LEAST_ONCE"`.

5. **Missing Token Exchange Service dependency**: Components accessing AWS services (S3, DynamoDB, etc.) must declare `aws.greengrass.TokenExchangeService` as a hard dependency in the recipe.

6. **Including AWS CLI in containers**: Don't install AWS CLI in Greengrass containers. Use static configuration with pre-fetched IoT endpoints instead — this reduces image size by ~67% and startup time by ~75%.

7. **Running Greengrass Nucleus as non-root**: The full Nucleus runtime requires root privileges in containers. Use `--init --privileged` flags. (Lite uses `ggcore` user instead.)

8. **Forgetting `flush=True` in Python print statements**: Component logs won't appear in systemd journal without explicit output flushing.

## Defaults

Use these unless the user specifies otherwise:
- **Container runtime**: Podman (preferred over Docker)
- **Region**: us-east-1
- **Working directory**: `/tmp/` for setup files and configurations
- **Component language**: Python
- **Deployment method**: Cloud deployment (preferred over local for full AWS experience)
- **Recipe format**: YAML (easier for local development than JSON)

## LLM Context Files

For up-to-date API and service documentation (covers both Nucleus and Lite):
- **Developer Guide**: https://docs.aws.amazon.com/greengrass/v2/developerguide/llms.txt
- **API Reference**: https://docs.aws.amazon.com/greengrass/v2/APIReference/llms.txt

## Reference Files

Load these as needed for detailed implementation guidance:

- [Setup Nucleus](./references/setup/setup-greengrass-container.md) — Set up Greengrass Nucleus (full Java runtime) in a container with root privileges
- [Setup Lite](./references/setup/setup-greengrass-lite-container.md) — Set up Greengrass Nucleus Lite (lightweight C runtime) in a container with systemd
- [Component Development](./references/components/component-development.md) — Create component recipes, IPC patterns, access control, and best practices
- [Component Recipe Reference](./references/components/comprehensive-component-recipe.yaml) — Complete recipe template with all supported fields
- [IoT Core Publisher](./references/components/create-iot-core-publisher-component.md) — Create components that publish to AWS IoT Core via IPC
- [Token Exchange Service](./references/components/token-exchange-service-guide.md) — Critical TES dependency for components accessing AWS services
- [Deployment](./references/deployment/deploy-components-to-greengrass-lite.md) — Deploy components via cloud or local methods, thing group setup, IAM permissions
- [Migration V1 to V2](./references/migration/migrate-v1-lambda-to-v2-component.md) — Migrate Greengrass V1 Lambda functions to V2 generic components
- [SDK Migration Reference](./references/migration/sdk-migration-reference.md) — SDK migration patterns for Python, Java, Node.js, C, and C++
- [Troubleshooting](./references/troubleshooting/common-issues.md) — Common issues, container best practices, debugging commands
- [Container Best Practices](./references/troubleshooting/container-best-practices.md) — Container security, static configuration, and performance optimization
- [Container Troubleshooting](./references/troubleshooting/container-troubleshooting.md) — Greengrass Lite container debugging and service recovery
- [Deployment Patterns](./references/deployment/deployment-patterns.md) — Deployment strategies, runtime selection, fleet deployment, and rollback
- [Cleanup Cloud Resources](./references/cleanup/cleanup-cloud-resources.md) — Safe cleanup of AWS IoT resources (things, certificates, policies, IAM roles)
- [Cleanup Local Resources](./references/cleanup/cleanup-local-resources.md) — Container and local file cleanup
