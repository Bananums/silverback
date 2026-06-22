# Silverback

## Planned upper level project structure ##

```text
silverback
├── README.md
├── agents
├── broker
├── context.md
├── contracts
├── deployment
├── docs
├── fleet
├── operator
├── robot
├── simulation
└── tools
```


```text
[Gamepad HW] → [Input Abstraction] → [Command Mapping] → [Broker]
                                              ↕
                                         contracts/
                                              ↕
                        [Broker] → [Command Dispatcher] → [HAL Interface]
                                                                ↕
                                                   [GPIO impl] | [Sim impl]
```

### Operator to Robot development start ###

```text
operator/
├── input/          ← abstract IInputDevice interface
│   └── gamepad/    ← concrete gamepad impl (SDL2, etc.)
└── publisher/      ← reads IInputDevice, maps to contracts, publishes
```

The key: `input/` defines an interface, `gamepad/` implements it. A future keyboard or VR controller is just another `input/` implementation. The publisher doesn't know what kind of input it's reading.

Contracts (the shared language):

```text
contracts/
└── command/        ← TwistCommand, JointCommand, etc.
```
Critically, publish semantic commands, not raw gamepad values. The operator maps axes → `TwistCommand{linear_x, angular_z}`. The robot never knows a gamepad existed. An autonomous agent can publish the exact same TwistCommand and the robot is indifferent to the source.

Robot side (RPi):

```text
robot/
├── hal/
│   ├── interface/  ← pure abstract: IMotorDriver, IGPIO, etc.
│   ├── rpi/        ← concrete RPi/GPIO implementation
│   └── sim/        ← simulation backend (delegates to simulation/)
├── control/        ← receives contracts, calls HAL interfaces only
└── node/           ← broker subscriber, wires control + HAL together
```
`control/` only ever calls `IMotorDriver::set_velocity()`. It has zero knowledge of GPIO registers, PWM, or even which robot it's running on. The HAL implementation is injected at startup in `node/`.

**Why this matters practically**

| Concern                               | Solved By                                        |
|---------------------------------------|--------------------------------------------------|
| Test control logic without hardware   | Inject a mock HAL                                |
| Run in simulation                     | Inject the sim HAL                               |
| Port to a different motor controller  | New HAL implementation, no control changes       |
| Autonomous agent replaces human       | Publishes same contracts, robot unchanged        |
| Swap Zenoh for something else         | Only `node/` changes                             |

**The swap point**
The HAL is the only place where the robot's software touches hardware. Everything above it is portable. This is what lets simulation be a first-class citizen in the architecture. `robot/hal/sim/` delegates to `simulation/`, and the entire control stack runs identically.

### Contracts for combinations

Exactly right. You've just described the full combination matrix:

| Input Side       | Robot HAL | Use Case                                      |
|------------------|-----------|-----------------------------------------------|
| `input/gamepad/`   | `hal/rpi/`  | Normal teleoperation                          |
| `input/gamepad/`   | `hal/sim/`  | Test operator UX without hardware             |
| `input/scripted/`  | `hal/rpi/`  | HIL — scripted commands, real hardware        |
| `input/scripted/`  | `hal/sim/`  | Virtual E2E, runs in CI with no hardware      |

The contracts are what make this work. Because both sides only speak contracts, neither side knows or cares what's on the other end. The swap points are:

**Operator side**: IInputDevice — gamepad, keyboard, scripted replay, or an AI agent
**Robot side**: HAL — RPi GPIO, simulation, or a mock for unit tests
And agents slot in for free. An autonomous agent just publishes the same TwistCommand contract that the gamepad publisher does. Thus resulting in:

| Input Provider | Target HAL | Use Case                                      |
|----------------|------------|-----------------------------------------------|
| nav            | sim        | Autonomous agent development without hardware |
| nav            | rpi        | Autonomous deployment                         |

The core (control/) never changes across any of these combinations. You wrote it once against the HAL interface and it runs everywhere. This is the architecture paying off.

This pattern has a name — **Ports and Adapters** (or Hexagonal Architecture). Your contracts are the ports, your HAL implementations and input implementations are the adapters. Worth knowing so you can look it up if you want to go deeper.