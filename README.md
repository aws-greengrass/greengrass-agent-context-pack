# AWS IoT Greengrass AI Agents Context Pack

This repository provides structured knowledge and step-by-step guides for AI agents to quickly set up and experiment with AWS IoT Greengrass Nucleus. It follows the [https://agents.md](https://agents.md) specification.

## ⚠️ Important Notice

**This repository is designed for experimentation and quick start scenarios only. It is NOT intended for production devices or environments.**

## Purpose

This context pack enables AI agents to:
- Set up Greengrass Nucleus (full runtime) and Greengrass Nucleus Lite (constrained devices) in containerized environments
- Create and deploy custom IoT components
- Follow best practices for component development
- Troubleshoot common issues
- Provide guided assistance for Greengrass Nucleus experimentation

## AI Agent Integration

AI agents should reference [AGENTS.md](./AGENTS.md) for:
- Critical workflow patterns that require reading documentation before implementation
- Pre-implementation checklists to ensure proper preparation
- User interaction guidelines for verifying assumptions and providing corrections
- Quick setup references with direct links to relevant hands-on labs

### Getting Started with the Agent

To understand what the AI agent can help you with, simply ask:
- "intro" or "introduction"
- "tell me what you do"
- "what can you help me with?"

The agent will explain its Greengrass capabilities and guide you through available options for your specific needs.

## Supported Greengrass Variants

### Greengrass Nucleus (Full Runtime)
Provides the complete IoT Greengrass experience with full component lifecycle management, advanced deployment capabilities, and comprehensive features suitable for development and testing environments.

### Greengrass Nucleus Lite (Constrained Devices)
Offers a lightweight runtime optimized for resource-constrained environments with a simplified component model, edge device optimization, and rapid experimentation capabilities.

## Getting Started

AI agents should follow this sequence:

1. **Review [AGENTS.md](./AGENTS.md)** for workflow patterns and implementation guidelines
2. **Select appropriate setup**:
   - For full feature set: `setup/setup-greengrass-container.md`
   - For constrained environments: `setup/setup-greengrass-lite-container.md`
3. **Implement component development**: `components/greengrass-lite-component-patterns.md`
4. **Execute deployment**: `setup/deploy-components-to-greengrass-lite.md`

## Technical Considerations

- **Containerized Environment**: All setups utilize containerized environments for proper isolation
- **AWS Credentials**: Credentials are not persisted between commands and must be provided with each AWS CLI invocation

## Usage Guidelines

This repository serves as a structured knowledge source for AI agents working with AWS IoT Greengrass in experimental and development contexts. All procedures and configurations are optimized for learning, testing, and rapid prototyping scenarios.
