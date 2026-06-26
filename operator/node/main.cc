#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>
#include <iostream>

#include "zenoh.hxx"

#include "contracts/command/twist.h"

int main() {
    auto session = zenoh::Session::open(zenoh::Config::create_default());
    auto pub = session.declare_publisher("silverback/robot/cmd/twist");
    
    std::cout << "Starting operator node" << std::endl;

    silverback::contracts::Twist twist{};
    
    while (true) {
        twist.timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        twist.linear_x = twist.linear_x + 1.0f;
        twist.angular_z = twist.angular_z + 0.25f;

        pub.put(zenoh::Bytes(std::vector<uint8_t>(
            reinterpret_cast<const uint8_t*>(&twist),
            reinterpret_cast<const uint8_t*>(&twist) + sizeof(twist)
        )));

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }

    std::cout << "Shutting down operator node" << std::endl;
}
