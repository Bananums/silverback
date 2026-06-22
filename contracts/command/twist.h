#ifndef SILVERBACK_CONTRACTS_COMMAND_TWIST_H
#define SILVERBACK_CONTRACTS_COMMAND_TWIST_H

#include <cstdint>

namespace silverback::contracts {

// Velocity command in 3D space.
// Differential drive robots use linear_x and angular_z. Unused axes are zero.
struct Twist {
    uint64_t timestamp_ns;
    float linear_x;
    float linear_y;
    float linear_z;
    float angular_x;
    float angular_y;
    float angular_z;
};

} // namespace silverback::contracts

#endif // SILVERBACK_CONTRACTS_COMMAND_TWIST_H
