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


## Control Plane vs Data Plane

Your mental model is almost exactly right. It is a triangle — but the three sides carry very different things:

```text
        Operator
        /      \
       /        \
   [control]  [data plane]
   [heartbeat] [commands]
     /          [telemetry]
    /            [video]
   /              [safety watchdog]
 Broker ——————— Robot
      [control]
      [heartbeat]

```
* **Operator ↔ Broker** — control plane only. Session requests, heartbeat, state updates. Thin channel.
* **Robot ↔ Broker** — control plane only. Registration, heartbeat, availability updates. Thin channel.
* **Operator ↔ Robot** — data plane P2P over WireGuard. Commands, telemetry, video, safety watchdog. This is the fat, latency-critical channel. The broker provisioned it but is completely absent from it.

The broker sides of the triangle stay alive after the session is established, but they are just heartbeats. The broker is not involved in anything that happens on the operator-robot link. If the broker went down mid-session, the P2P link would keep running, the session would continue until the next heartbeat timeout.

That is actually the robustness property. In a teleoperation system, the control plane going down should drop an active session.