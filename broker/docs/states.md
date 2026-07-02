# Broker Docs #
Docs describing the design and architecture of the broker system... TODO fill out summary/introduction

## Current Development State ##

Comment: Good foundation, but it has two significant gaps: the states are modeled as linear sequences when they should be graphs with branches, and the failure paths are completely absent. Here's what needs to be added:

Missing states:

Robot needs unavailable — registered but not accepting sessions (robot internally in RECOVERY, etc.). Without it there's no way to model a robot that's reachable but refusing sessions.
Session needs terminating — graceful shutdown is a distinct state from abrupt termination.
Missing failure paths:

Every state needs an exit path on failure, not just the happy path:

Any robot state → disconnected on heartbeat timeout
Session negotiating → what? If robot drops mid-negotiation the session can't just hang
Session active → what happens on ungraceful robot disconnect vs. operator disconnect?
Missing transition events:

The states are listed but what drives each transition is absent. That's where the race conditions live. For example:

What event moves robot from available to in-session? When exactly — when the session is negotiating, or when it is active?
If two operators request the same robot simultaneously, which event wins and what happens to the other?


## State Machines

The broker tracks three state machines independently. Transition diagrams for each
are in the accompanying drawio files.

---

### Robot State

- **`unregistered`** — Broker has no record of this robot.
    - sldkf
- **`registered`** — Robot has authenticated and announced itself. Not yet accepting sessions.
- **`available`** — Robot is ready and accepting session requests.
- **`unavailable`** — Robot is reachable but not accepting sessions. Driven by the robot itself (e.g. internal recovery, maintenance mode).
- **`in_session`** — Robot is in an active session with an operator.
- **`disconnected`** — Robot was previously known but heartbeat has timed out.

See [robot-states.drawio](robot-states.drawio)

![Robot state machine](robot-states.svg)

#### Lifecycle

The broker maintains one state machine instance per robot. A new instance starts at
`unregistered` the first time the broker sees a robot, either on broker startup (no persistence gap) or on the first ever connection from a previously unknown robot ID.

`unregistered` is only reachable as an entry point. Once a robot transitions to
`registered`, it never returns to `unregistered` for the lifetime of that broker
instance. If the broker restarts without persistence, all instances are lost and
robots must re-register from scratch.

`disconnected` and `unregistered` are distinct: `disconnected` means the broker
still has a record of the robot but its heartbeat has timed out. Reconnection takes
the robot to `registered`, not back to `unregistered`, because the broker
remembers it. This distinction matters for policy: a reconnecting robot can skip
any one-time provisioning steps.

`unavailable` is robot-driven. The broker does not impose it; the robot signals it
(e.g. entering an internal recovery routine or maintenance mode). The broker honors
it by refusing session requests until `recovered` is signalled.

There is no terminal state. `disconnected` is always recoverable.

---

### Operator State

- **`unregistered`** — Broker has no record of this operator.
- **`connected`** — Operator has connected and authenticated.
- **`available`** — Operator is ready to request a session.
- **`requesting`** — Operator has submitted a session request, awaiting negotiation.
- **`in_session`** — Operator is in an active session with a robot.
- **`disconnected`** — Operator connection lost.

See [operator-states.drawio](operator-states.drawio)

![Robot state machine](operator-states.svg)

#### Lifecycle

The broker maintains one state machine instance per operator, created at `unregistered`
on first contact. The lifecycle mirrors the robot's but reflects that operators are
client applications, not physical devices.

`connected` is the operator equivalent of the robot's `registered`. The operator has
authenticated with the broker but has not yet signalled readiness. The naming differs
deliberately: robots register hardware identity and capabilities; operators simply
connect and authenticate. `ready_signal` advances the operator to `available` in both
cases.

`requesting` is the operator's unique waiting state. It exists between submitting a
session request and the broker resolving it. The broker is running ACL checks (Access Control Lists) and
coordinating with the robot. A denial returns the operator to `available` via
`session_denied`, which is a normal policy outcome, not a failure. The operator can
immediately request a different robot or retry.

There is no `unavailable` state. Operators do not enter maintenance or recovery modes.
An operator is either connected and participating, or disconnected.

`heartbeat_timeout` applies symmetrically: if the broker stops receiving keep-alives
from the operator client, it treats the operator as disconnected. This mirrors the
robot heartbeat, both sides of a session are held to the same liveness contract.

`disconnected` → `connected` on `reconnected`, the broker remembers the operator and
skips `unregistered` on reconnect, same as the robot.

There is no terminal state. `disconnected` is always recoverable.

---

### Session State

- **`requested`** — Operator has requested a session with a specific robot.
- **`negotiating`** — Broker is coordinating the connection (ACL check, WireGuard key exchange).
- **`established`** — Keys exchanged and P2P link configured. Session is ready.
- **`active`** — Operator and robot are communicating over the P2P link. Broker is no longer in the data path.
- **`terminating`** — Graceful shutdown initiated by either party.
- **`terminated`** — Session has ended. Final state.

See [session-states.drawio](session-states.drawio)

![Session state machine](session-states.svg)

#### Lifecycle

The session state machine is a joint entity, it represents the relationship between
one operator and one robot, not either participant individually. A new instance is
created for each session request and destroyed at a terminal state. Unlike robot and
operator instances which persist across reconnections, there is no recovery from a
terminal session state, a new request creates a new instance.

The session does not exist until ACL clears. `requested` is the entry point, and
`acl_check_passed` is the first gate. A denial at this stage goes directly to
`terminated failure`, there is nothing to tear down.

`negotiating` is the only phase where the broker is actively in the data path.
WireGuard key exchange and P2P link configuration happen here. The broker is
coordinating between robot and operator and holds the session open until keys are
exchanged.

`established` means the P2P link is configured but not yet confirmed live. The broker
hands off key material and waits for both sides to confirm the link is up. If
confirmation never arrives, `link_timeout` terminates the session.

`active` is the steady state. The broker exits the data path entirely — robot and
operator communicate P2P. The broker still holds the session record and monitors
liveness via control plane heartbeats. `heartbeat_timeout` here means the broker has
lost sight of one or both participants, not that the data link has failed.

`terminating` exists because graceful shutdown takes time. Both sides must
acknowledge, clean up WireGuard configuration, and confirm teardown. It is a
time-boxed state: `termination_timeout` moves to `terminated failure` if teardown
stalls, preventing the session from hanging indefinitely.

The two terminal states carry different policy weight. `terminated planned` signals a
clean end, reconnection attempts can be accepted immediately. `terminated failure`
signals something broke, policy may enforce a cooldown, trigger an alert, or require
operator acknowledgement before a new session to the same robot is allowed.

## Paths ##

the robot and opeartor states kind of lives by themselves they they are driven by and can drive the session state

Happy path:
```text
Robot:    [available] ————————————— [in_session] ————————— [available]
Session:             [requested] → [negotiating] → [active] → [terminated]
Operator: [available] ————————————— [in_session] ————————— [available]
```

Robot disconnects during active session:
```text
Robot:    [in_session] → [disconnected] ——→ [registered] → [available]
Session:  [active]     → [failed]
Operator: [in_session] → [available]   (failure policy triggers)
```
