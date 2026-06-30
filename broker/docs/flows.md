* 
* Transient failure (brief network hiccup) → reconnect policy automatically requests a new session
* Operator-initiated close → no automatic reconnect, operator decides
* Robot enters RECOVERY internally → robot marks itself unavailable in its own state machine, blocking new session requests until it's ready

Exactly right — and you've just naturally discovered the key flow that belongs in flows.md.

The clean separation is:

**Broker** — detects heartbeat_timeout, transitions session to terminated failure, sends failure notification to operator with the reason

**Operator** — receives notification, consults reconnect policy based on reason, decides whether to request a new session
The reason matters for the policy. For example:

| Failure reason      | Operator policy                                                   |
| ------------------- | ----------------------------------------------------------------- |
| `heartbeat_timeout` | Retry up to 5 times with backoff                                  |
| `acl_denied`        | Do not retry — this is a permissions issue, not a transient fault |
| `robot_unavailable` | Wait and poll, not immediate retry                                |
| `link_timeout`      | Retry once, then escalate                                         |

This also reveals something important: the **broker must include the failure reason in its notification** . That's a concrete Protobuf field you'll need — `SessionTerminatedNotification { reason: FailureReason, session_id: ... }`. Without the reason, the operator can't make a policy decision.

This is exactly the content of `flows.md` — the happy path flow, the `heartbeat_timeout` failure flow showing broker notifying operator and operator retrying, and the `acl_denied` flow showing why retry is suppressed. Those three scenarios alone cover the most critical behaviors and would satisfy a due diligence reviewer.
