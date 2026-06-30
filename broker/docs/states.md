# Broker Docs #
Docs describing the design and architecture of the broker system... TODO fill out summary/introduction

## Current Development State ##

Good foundation, but it has two significant gaps: the states are modeled as linear sequences when they should be graphs with branches, and the failure paths are completely absent. Here's what needs to be added:

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
- **`registered`** — Robot has authenticated and announced itself. Not yet accepting sessions.
- **`available`** — Robot is ready and accepting session requests.
- **`unavailable`** — Robot is reachable but not accepting sessions. Driven by the robot itself (e.g. internal recovery, maintenance mode).
- **`in_session`** — Robot is in an active session with an operator.
- **`disconnected`** — Robot was previously known but heartbeat has timed out.

See [robot-states.drawio](robot-states.drawio)

---

### Operator State

- **`unregistered`** — Broker has no record of this operator.
- **`connected`** — Operator has connected and authenticated.
- **`available`** — Operator is ready to request a session.
- **`requesting`** — Operator has submitted a session request, awaiting negotiation.
- **`in_session`** — Operator is in an active session with a robot.
- **`disconnected`** — Operator connection lost.

See [operator-states.drawio](operator-states.drawio)

---

### Session State

- **`requested`** — Operator has requested a session with a specific robot.
- **`negotiating`** — Broker is coordinating the connection (ACL check, WireGuard key exchange).
- **`established`** — Keys exchanged and P2P link configured. Session is ready.
- **`active`** — Operator and robot are communicating over the P2P link. Broker is no longer in the data path.
- **`terminating`** — Graceful shutdown initiated by either party.
- **`terminated`** — Session has ended. Final state.

See [session-states.drawio](session-states.drawio)
