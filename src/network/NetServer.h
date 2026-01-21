#pragma once

#include "NetCommon.h"
#include "NetProtocol.h"
#include <functional>
#include <vector>

namespace net {

class NetServer {
public:
    NetServer();
    ~NetServer();

    bool start(std::uint16_t port, std::size_t maxClients = 8);
    void stop();

    // Poll incoming events; timeoutMs can be 0 for non-blocking
    void service(int timeoutMs,
                 const std::function<void(ENetPeer*)>& onConnect,
                 const std::function<void(ENetPeer*)>& onDisconnect,
                 const std::function<void(ENetPeer*, const ENetPacket*)>& onPacket);

    bool broadcast(const std::vector<std::uint8_t>& data, bool reliable = false);
    bool sendTo(ENetPeer* peer, const std::vector<std::uint8_t>& data, bool reliable = false);

    ENetHost* rawHost() { return host; }

private:
    ENetContext ctx;
    ENetHost* host{nullptr};
};

} // namespace net
