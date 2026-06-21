# Silverback Context

## Overview

Silverback is a robotics teleoperation and fleet-management platform.

The platform enables operators, autonomous agents, and external systems to communicate with robots, industrial machines, and other edge devices through a brokered communication architecture.

Silverback is designed to support:

* Human teleoperation
* Remote monitoring
* Autonomous robot behaviors
* Fleet management
* Multi-robot systems
* Simulation environments
* Future web and cloud deployments

The platform is transport-agnostic at the architectural level but currently plans to use Zenoh as the underlying communication fabric.

---

## Design Philosophy

### Platform First

Silverback is a platform, not a single application.

Every component should be designed as an independent system element that can be developed, tested, deployed, and replaced independently.

### Flat Repository Structure

The repository should remain as flat as possible.

Avoid deep nesting and organizational folders such as:

* apps/
* services/
* libs/
* modules/
* packages/

Prefer meaningful top-level domains instead.

Good:

robot/
operator/
fleet/

Avoid:

apps/operator/
apps/fleet/
services/robot/

The top-level structure should communicate system architecture directly.

### Capability-Oriented Organization

Repository structure should reflect system capabilities rather than implementation technologies.

Organize around concepts such as:

* operator
* robot
* fleet
* contracts
* broker

Avoid organizing around frameworks:

* qt
* react
* rust
* python

Frameworks are implementation details and may change over time.

---

## Core Architecture

Silverback consists of several major domains.

### Contracts

Contracts are the shared language of the platform.

All communication between components should be defined through contracts.

Examples:

* Commands
* Telemetry
* Events
* State
* Capabilities
* Video metadata
* Fleet information

Contracts should be the source of truth for system interoperability.

---

### Broker

The broker provides communication infrastructure.

Current implementation is expected to use Zenoh.

Responsibilities include:

* Discovery
* Routing
* Message transport
* Identity
* Connectivity

Higher-level services should not depend directly on Zenoh-specific concepts whenever possible.

---

### Operator

Operator-facing interfaces.

Initially expected to use Qt6.

Future web-based interfaces should be supported without major architectural changes.

The operator domain should interact with contracts and platform capabilities rather than robot-specific implementations.

---

### Robot

Runtime components deployed on robots and industrial machines.

Robot systems should:

* Connect to the broker
* Advertise capabilities
* Publish telemetry
* Accept commands
* Report health

Robots should appear as platform nodes.

---

### Fleet

Fleet management capabilities.

Examples:

* Robot registry
* Monitoring
* Scheduling
* Provisioning
* Mission assignment
* Status visualization

Fleet functionality should support both teleoperated and autonomous systems.

---

### Agents

Autonomous and semi-autonomous components.

Examples:

* Navigation agents
* Planning agents
* Vision agents
* Inspection agents
* AI-assisted operators

Agents interact through contracts just like human operators.

**Open question:** Agent runtime location is not yet decided.

* On-robot (offline): agents run locally on the robot hardware
* Cloud-hosted (online): agents run on cloud infrastructure and communicate via the broker

The architecture should support both without major changes to contracts or robot components.

---

### Simulation

Simulation environments used for development, testing, and validation.

Simulated systems should be treated as first-class citizens within the platform.

---

## Long-Term Vision

Silverback should evolve from a teleoperation platform into a general robotics operating platform.

The architecture should support:

1. Single robot teleoperation
2. Multi-robot teleoperation
3. Fleet management
4. Autonomous systems
5. Human-agent collaboration
6. Cloud-connected robotics infrastructure

Without requiring major restructuring of the repository.

---

## Repository Structure

Preferred structure:

```text
silverback/
├── contracts/
├── broker/
├── operator/
├── robot/
├── fleet/
├── agents/
├── simulation/
├── tools/
├── deployment/
└── docs/
```

This structure reflects architectural domains rather than implementation details.

### tools/

`tools/` at the repository root is reserved for project-wide tooling only.

Examples:

* Bazel macros and build rules
* Code generation scripts
* Developer setup scripts
* CI/CD helpers

Individual domains may contain their own `tools/` subdirectory for tooling that is only relevant to that domain.

### Testing

Tests live alongside the code they cover, within the domain that implements them.

Example:

```text
robot/
└── hal/
    └── tests/
```

There is no top-level `tests/` directory. This keeps tests close to their implementation and avoids a parallel directory tree.

---

## Build System

Silverback uses Bazel as the monorepo build system.

Bazel provides:

* Hermetic, reproducible builds across languages
* Fine-grained dependency management between domains
* Support for C++, Rust, and Python in a single build graph
* Remote caching and distributed builds for CI

Each domain owns its own `BUILD` files. Shared Bazel macros and rules live in `tools/`.

---

## Languages

The primary implementation language is C++.

Rust may be used where it offers a clear advantage, such as:

* Safety-critical subsystems
* High-performance networking or concurrency
* Components that benefit from Rust's ownership model

Python may be used for:

* Scripting and tooling
* Rapid prototyping of agent behaviors
* Data analysis and visualization

Language choice should be driven by the requirements of the component, not consistency for its own sake.

---

## Naming Principles

Prefer:

* Short names
* Clear names
* Singular nouns
* Architectural meaning

Examples:

* robot
* operator
* fleet
* broker
* contracts

Avoid:

* unnecessary abbreviations
* framework-specific names
* implementation-specific naming

The repository should remain understandable to a new engineer after several years of development.

---

## Guiding Question

Before introducing a new component, ask:

"Is this a new architectural domain, or simply an implementation detail of an existing domain?"

Create new top-level directories only for new architectural domains.
