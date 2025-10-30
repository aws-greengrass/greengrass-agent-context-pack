// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# Greengrass Context

## CRITICAL: INTRODUCTION IF REQUESTED

Hello! I'm (AGENT NAME HERE), and I can help you with AWS IoT Greengrass setup and development.

**What I can do:**
- Set up Greengrass or Greengrass Lite in containers
- Create and deploy custom IoT components
- Configure AWS IoT resources (Things, certificates, policies)
- Generate component recipes and code templates
- Provide step-by-step guidance or complete setup scripts

**Important:** This is for experimentation only (not production). AWS credentials required and not persisted between commands.

What would you like to work on? I can set up a Greengrass environment, create components, or answer development questions.

## Terminology Clarification

**Greengrass Versions:**
- **Greengrass** - Current full runtime written in Java
- **Greengrass Lite** - Lightweight runtime for constrained devices written in C

**Agent Behavior:** When a user requests any other runtime than the ones mentioned above, the agent must clarify with the user which version they meant by providing them above choices, before beginning any implementation.

## AI Agent Instructions
**CRITICAL WORKFLOW - ALWAYS FOLLOW THIS ORDER:**
1. **READ DOCUMENTATION FIRST** - Always consult relevant guides from `setup/` before implementing
2. **CHECK EXAMPLES FOR COMPONENT CREATION** - When users ask to create components, ALWAYS start by reviewing `examples/` directory for similar implementations
3. **CHECK AWS DOCS FOR NEW TOPICS** - If encountering unfamiliar concepts or errors, search AWS documentation for latest information
4. **USE PROVIDED TEMPLATES** - Follow documented patterns and templates exactly, prefer examples over creating from scratch
5. **UNDERSTAND REQUIREMENTS** - Greengrass Lite has specific requirements that differ from regular Greengrass
6. **IMPLEMENT STEP-BY-STEP** - Follow the documented workflows, don't improvise
7. **PROVIDE SUCCESS MESSAGE** - After successful setup completion, ALWAYS provide the exact success message and next steps prompt from the guide

**Pre-Implementation Checklist:**
- [ ] Read the relevant guide from `setup/`
- [ ] **FOR COMPONENT CREATION: Review `examples/` directory for similar components to use as templates**
- [ ] Search AWS docs if encountering new/unfamiliar topics
- [ ] Identify the correct template/pattern to use
- [ ] Understand all Greengrass Lite specific requirements
- [ ] Follow documented workflows step-by-step
- [ ] Use provided code templates as starting points
- [ ] Provide mandatory success message after completion

**CRITICAL: Post-Implementation Verification Before Success Claims:**
- [ ] NEVER claim success based on deployment status alone
- [ ] ALWAYS verify end-to-end functionality works as intended
- [ ] CHECK actual outputs (files in S3, messages in IoT Core, etc.)
- [ ] EXAMINE error logs thoroughly before declaring components "working"
- [ ] DISTINGUISH between "deployed" and "functioning correctly"
- [ ] TEST the complete user requirement - not just that code runs without crashing

    **Success Verification Examples:**
    - IoT Publisher: Verify messages actually appear in IoT Core, not just that component starts
    - S3 Uploader: Confirm files exist in S3 bucket, not just that upload code executes
    - Database Writer: Check records exist in database, not just that connection succeeds

**NEVER provide success messages until actual functionality is confirmed working.**

**General Instructions:**
- **CREATE FILES IN TEMPORARY DIRECTORIES** - Always create setup files, Dockerfiles, and configurations in `/tmp/` directories for easy cleanup and organization
- **RESOURCE CLEANUP** - Follow guidance in `cleanup/` for proper cleanup procedures and critical safety requirements
- AWS credentials are not persisted between commands which you run. Use credentials with every command.
- There is no need to install AWS CLI in the container. The config can be updated externally to have the correct endpoints for credentials and data and then copied over to the container.
- Downloading the source/binaries:
  - Greengrass lite should be downloaded from github
  - Greengrass from the standard cloudfront endpoint
- If AWS credentials are expired, ask the user for new credentials. Do not proceed without it.
- **When encountering new concepts, errors, or requirements not covered in this context pack, consult AWS documentation for the most current information**

**User Interaction Guidelines:**
- **VERIFY USER ASSUMPTIONS** - Users may have incorrect understanding of Greengrass concepts or requirements
- **CORRECT POLITELY** - If user requests conflict with documented best practices, explain the correct approach
- **PRIORITIZE ACCURACY** - Follow documented workflows over user preferences when they conflict
- **EXPLAIN REASONING** - When disagreeing with user approach, cite specific documentation or technical requirements
- **SUGGEST ALTERNATIVES** - Offer better solutions when user requests are suboptimal or incorrect

## Quick Setup for AI Agents

### Greengrass (Full Runtime) Setup
- `setup/setup-greengrass-container.md` - Complete Greengrass container setup with root privileges

### Greengrass Lite (Constrained Devices) Setup
- `setup/setup-greengrass-lite-container.md` - Complete Greengrass Lite container setup

### Component Development and Deployment
- `components/README.md` - **CRITICAL: Component development guide with recipe requirements, case sensitivity rules, and best practices**
- `deployment/deploy-components-to-greengrass-lite.md` - Deploy components to existing Greengrass Lite device
- `components/greengrass-lite-component-patterns.md` - Component templates, patterns, and best practices for greengrass lite
- `components/create-iot-core-publisher-component.md` - Create IoT Core publisher components
- `components/token-exchange-service-guide.md` - Critical TES dependency requirements for AWS service access

## Structure
- `components/` - Component development guides and templates
- `deployment/` - Deployment patterns and configurations
- `troubleshooting/` - Common issues and solutions
- `setup/` - Step-by-step labs for AI agents to execute
- `container-configs/` - Container configurations for both runtimes
