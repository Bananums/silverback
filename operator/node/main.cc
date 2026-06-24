#include <thread>
#include <vector>

#include "zenoh.hxx"

int main() {
    auto session = zenoh::Session::open(zenoh::Config::create_default());
    auto pub = session.declare_publisher("silverback/robot/cmd/twist");

    while (true) {
        std::string msg = "twist:0.5,0.0";
        pub.put(zenoh::Bytes(std::vector<uint8_t>(msg.begin(), msg.end())));
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}
