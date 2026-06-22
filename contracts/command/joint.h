#ifndef SILVERBACK_CONTRACTS_COMMAND_JOINT_H
#define SILVERBACK_CONTRACTS_COMMAND_JOINT_H

#include <cstdint>


namespace silverback::contracts {

static constexpr uint8_t kMaxJoints{8};

enum class JointMode : uint8_t {
    position = 0,
    velocity = 1,
    torque = 2
};

struct JointControl {
   float value{};
   JointMode mode{};
};

struct Joint
{
    uint64_t timestamp_ns{};
    uint8_t count{};                   //< The amount of available joints the machine has 
    JointControl control[kMaxJoints];  //< The control of a specified joint
};

} //silverback::contracts

#endif //SILVERBACK_CONTRACTS_COMMAND_JOINT_H