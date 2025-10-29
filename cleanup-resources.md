// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Cleanup Resources Guide

## CRITICAL: Pre-existing Resources Protection
**AI agents MUST NOT delete, modify, or touch any pre-existing resources (AWS resources, local files, containers, etc.) that were not created during the current session.**

## AWS Greengrass Components

**Component Deletion:**
- MUST specify full versioned ARN: `--arn "arn:aws:greengrass:region:account:components:ComponentName:versions:Version"`
- Deleting without version specification WILL fail with "Request component version is blank"
- SHOULD delete S3 artifacts after component deletion

**Deployment Cleanup:**
- MAY use empty deployment to remove components: `--components "{}"`
- SHOULD cancel active deployments before resource deletion

## AWS IoT Resources

**Deletion Order (MUST follow):**
1. Detach policies from certificates
2. Detach certificates from things
3. Deactivate certificates
4. Delete certificates, policies, things, thing groups

**IAM Resources:**
- MUST delete role alias before IAM role
- MUST detach policies before deleting IAM role

## EC2 Resources

**Critical Order:**
- MUST terminate instances before deleting security groups
- SHOULD wait for instance termination to complete
- MAY delete key pairs after instance termination

## S3 Resources

- MUST delete bucket contents before deleting bucket
- SHOULD use `--recursive` flag for multi-object deletion

## Local Resources

**Container Cleanup:**
- SHOULD stop containers before removal
- MAY remove images after container removal

**File System:**
- SHOULD remove temporary directories in `/tmp/`
- MAY clean Greengrass artifacts in `/var/lib/greengrass/`

## Key Learnings

- Component deletion requires versioned ARN (not component name alone)
- Resource dependencies MUST be respected (certificates before things, etc.)
- EC2 security groups cannot be deleted while instances are running
- S3 buckets cannot be deleted while containing objects
