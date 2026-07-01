# Broker #

**TODO** make introduction.

## Design Layers ##
The broker is designed across three orthogonal layers

* State machines - what happens (broker logic)
* Protobuf schemas - what the messages contain (contracts)
* Transport - how messages travel (WireGuard / TLS / Zenoh)

The **state machine** defines what happens. States, transitions, failure paths. It does not care how the messages travel to drive those transitions.

**WireGuard vs TLS** is how messages travel. The transport layer. The session_requested event reaches the broker whether it came over a WireGuard tunnel, TLS, or a plain TCP socket. The broker's state machine does not know or care.

Each layer can be designed and changed independently. For example entire state machine design and flows documentation can be finished, and the WireGuard vs TLS decisions can be made later, without touching a single state or transition.

The only place the transport decision leaks into the broker design is the session establishment flow. Specifically the step where WireGuard public keys are exchanged. That step exists in the flow regardless of which transport you choose. The mechanism differs, the flow structure does not.

## Deployment Tiers ##

Exactly — and you've just articulated the three deployment tiers the broker needs to support:

Tier 1 — Local / development
Single broker process, Zenoh multicast discovery, auth off. A developer laptop. Zero configuration overhead.

Tier 2 — Site / medium WAN
Single broker instance at a known IP, token auth, handful of operators and robots. A small fleet operation or a demo environment.

Tier 3 — Cloud / enterprise
Kubernetes deployment, multiple broker instances behind a load balancer, shared state store, mTLS + ACL, horizontal scaling. Hundreds of robots, multiple concurrent sessions.

What makes this work across all three is exactly what you said: the broker is standalone. The same binary, different configuration. The robot and operator don't know or care which tier the broker is running in — they just connect to an endpoint and speak the protocol.

The thing that unlocks Tier 3 specifically is the persistence gap from the due diligence review — the shared state store. In Tier 1 and 2, broker state can live in memory. In Tier 3, multiple broker instances need to share registry and session state, so it moves to something like Redis or etcd. But that's an internal broker implementation detail. Robot and operator code doesn't change.

This is worth capturing in broker.md as a deployment tiers section — it directly answers the investor question "how does this scale?" with a concrete, graduated answer rather than a vague claim.

## Treadeoff ##
Architectual tradeoff: known architectural tradeoff that maps directly.

| Broker state            | What works                        | What doesn't                                            |
| ----------------------- | --------------------------------- | ------------------------------------------------------- |
| Up                      | Everything                        | —                                                       |
| Down, session active    | P2P link continues, commands flow | New sessions, graceful disconnect via broker, reconnect |
| Down, no active session | Nothing                           | Can't establish any session                             |

An existing active session survives broker downtime. that is the robustness property targeted during teleoperation. However, the moment that session ends (either the operator closes it locally or the safety watchdog fires), the connection cannot be re-establish until the broker is back.

For local LAN development with a single operator and robot, this is completely acceptable. The broker going down is recoverable by restarting it.

For production/SaaS, this is why the HA gap matters. A clustered broker with a shared state store means broker restarts are transparent. Sessions persist, new requests keep working. That is what "infinite software scaling" actually requires behind the scenes.

The good news is your architecture already supports adding HA later without changing the robot or operator at all — they just reconnect to whichever broker instance is alive. The HA complexity is entirely contained inside the broker domain.