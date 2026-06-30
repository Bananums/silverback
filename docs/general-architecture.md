# General Architecture

Silverback is structured as a set of independently deployable platform disciplines.
Each discipline is designed to be viable as a standalone service — usable in
isolation, composable with the others, and replaceable without affecting the rest
of the platform.

## Platform Disciplines

| Discipline | Role | As a standalone service |
|---|---|---|
| Broker | Control plane | Identity, access control, and session management offered as managed infrastructure |
| Robot | Edge software stack | SDK licensed to hardware manufacturers; HAL-adapted per target platform |
| Operator | Human-facing interface | Application or white-labeled client for third-party deployments |
| Fleet | Management layer | Monitoring, scheduling, and provisioning SaaS consuming broker registry and telemetry |

Together they form a complete teleoperation and fleet management platform. Individually,
each is a self-contained product with its own API surface and deployment model.

This mirrors models proven at scale: Android licenses an OS layer that OEMs adapt
to their hardware; VxWorks is an RTOS licensed to hardware manufacturers;
commercial fleet management layers sit above open robotics stacks the same way
Fleet sits above the Broker. The difference is that all four disciplines here share
a single coherent protocol rather than integrating across organizational boundaries.

## Architectural Properties

**Domain independence** — each discipline has a defined API boundary. Internal
implementation changes do not affect other disciplines as long as the API contract
is preserved.

**Protocol ownership** — the broker owns and versions the control plane protocol.
Participants implement against it; they do not define it.

**HAL abstraction** — the robot stack separates platform behavior from hardware
specifics. The same software runs on simulation, development hardware, and production
targets by swapping the HAL. A hardware manufacturer integrating the robot stack
provides only a HAL implementation — everything above it is included.

**Deployment agnosticism** — topology differences (local LAN, site network, cloud)
are configuration, not code. The same binaries run in all environments.
