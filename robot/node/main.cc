#include <cstring>
#include <iostream>
#include <thread>
#include <iostream>

#include "zenoh.hxx"
#include "contracts/command/twist.h"

int main(){
    std::cout << "Starting robot node" << std::endl;

    auto session = zenoh::Session::open(zenoh::Config::create_default());

    const char* topic_name{"silverback/robot/cmd/twist"};


    zenoh::ZResult err;

    auto sub = session.declare_subscriber(
            topic_name,
            [](const zenoh::Sample& sample) {
                auto bytes = sample.get_payload().as_vector();

                if (bytes.size() != sizeof(silverback::contracts::Twist)) {
                    std::cerr << "unexpected payload size: " << bytes.size() << "\n";
                    return;
                }

                silverback::contracts::Twist twist;
                std::memcpy(&twist, bytes.data(), sizeof(twist));

                std::cout << "recv twist:"
                        << " linear_x="  << twist.linear_x
                        << " angular_z=" << twist.angular_z
                        << " ts="        << twist.timestamp_ns
                        << "\n";
            },
            []() {}, // on_drop, do nothing
            zenoh::Session::SubscriberOptions{},
            &err
        );

    std::cout << "Robot node listening on silverback/robot/cmd/twist\n";
    while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }

    std::cout << "Stopping robot node" << std::endl;

    return 0;
}
