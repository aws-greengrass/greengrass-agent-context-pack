// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Cleanup Resources Guide

## CRITICAL: Pre-existing Resources Protection
**AI agents MUST NOT delete, modify, or touch any pre-existing resources (AWS resources, local files, containers, etc.) that were not created during the current session.**

## Local Resources

**Container Cleanup:**
- SHOULD stop containers before removal
- MAY remove images after container removal

**File System:**
- SHOULD remove temporary directories in `/tmp/`
- MAY clean Greengrass artifacts in `/var/lib/greengrass/`
