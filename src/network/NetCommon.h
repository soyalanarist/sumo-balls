#pragma once

#include <enet/enet.h>
#include <stdexcept>
#include <atomic>

namespace net {

class ENetContext {
public:
    ENetContext() {
        if (initCount.fetch_add(1) == 0) {
            if (enet_initialize() != 0) {
                initCount.fetch_sub(1);
                throw std::runtime_error("Failed to initialize ENet");
            }
        }
    }

    ~ENetContext() {
        if (initCount.fetch_sub(1) == 1) {
            enet_deinitialize();
        }
    }

    ENetContext(const ENetContext&) = delete;
    ENetContext& operator=(const ENetContext&) = delete;

private:
    static std::atomic<int> initCount;
};

} // namespace net
