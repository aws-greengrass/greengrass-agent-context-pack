// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: CC-BY-SA-4.0

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

See [AGENTS.md](AGENTS.md) for full workflow patterns, implementation guidelines, and content structure.

## Quick Reference

- This repository provides structured knowledge for AI agents to set up and experiment with AWS IoT Greengrass in containerized environments
- **Not for production** — experimentation and quick start scenarios only
- Two Greengrass variants: **Nucleus** (full Java runtime) and **Lite** (lightweight C runtime for constrained devices)
- Always read the relevant guide from `setup/` before implementing
- For component creation, always check `examples/` for similar implementations first
- Recipe fields are **case-sensitive** for Greengrass Lite — see `components/README.md`

## LLM Context Files

For up-to-date API and service documentation beyond what this repository covers (includes both Nucleus and Lite):

- **Developer Guide**: https://docs.aws.amazon.com/greengrass/v2/developerguide/llms.txt
- **API Reference**: https://docs.aws.amazon.com/greengrass/v2/APIReference/llms.txt
