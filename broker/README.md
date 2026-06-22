Use well-known pattern called control plane / data plane separation:

Control plane (broker): identity, auth, registry, session negotiation — low frequency, latency-tolerant
Data plane (P2P): commands, telemetry, video — high frequency, latency-critical
The broker is never in the hot path for actual robot data. It just decides who is allowed to talk to who, then gets out of the way.

Zenoh supports this well. Zenoh's router (zenohd) handles peer discovery and can enforce access control. Once two peers have found each other through the router, Zenoh can route their traffic directly without the router being a bottleneck — or they can go fully P2P without it.

This means the broker domain actually has two distinct responsibilities worth separating internally:

```text
broker/
├── registry/    ← which robots exist, their capabilities, availability
├── auth/        ← identity, operator permissions, ACL
└── session/     ← connection negotiation, establishes the P2P link
```

And the flow becomes:

1. Robot boots → registers with broker (identity + capabilities)
2. Operator connects → authenticates with broker
3. Operator requests robot → broker checks ACL → signals both to establish P2P
4. P2P link up → all command/telemetry flows directly, broker not involved
This also sets up fleet management naturally — fleet/ is essentially a view over the broker's registry. It knows what's registered, what's connected, what's in use.

### Do not forget about DSCP ###
So the real networking concern for this system isn't command efficiency — it's making sure video doesn't crowd out commands when the link is under pressure. That's a Zenoh QoS and potentially a network DSCP marking question for later, but it shapes how you think about the two traffic classes from the start.
