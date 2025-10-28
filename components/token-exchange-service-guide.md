# Token Exchange Service (TES) Component Guide

## Critical Requirements for AWS Service Access
When creating Greengrass components that need to access AWS services (S3, IoT Core, etc.), you MUST declare TES as a component dependency.

## Required Configuration
```yaml
ComponentDependencies:
  aws.greengrass.TokenExchangeService:
    VersionRequirement: ">=0.0.0"
    DependencyType: HARD
```
**Correct approach:**
```yaml
ComponentDependencies:
  aws.greengrass.TokenExchangeService:
    VersionRequirement: ">=0.0.0"
    DependencyType: HARD

Manifests:
  - Platform:
      os: linux
    Lifecycle:
      Run: "python3 script.py"
```

## IAM Role Requirements
The `GreengrassV2TokenExchangeRole` must have appropriate AWS service permissions:
- S3 access for file uploads
- IoT Core permissions for publishing
- Other service permissions as needed

## Troubleshooting
**Error: "Unable to locate credentials"**
- Add TES component dependency
- Ensure IAM role has required service permissions
