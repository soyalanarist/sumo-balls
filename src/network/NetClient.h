#pragma once

#include "NetCommon.h"
#include "NetProtocol.h"
#include <functional>
#include <vector>
#include <string>

namespace net {

class NetClient {
public:
    NetClient();
    ~NetClient();

    bool connect(const std::string& host, std::uint16_t port);
    void disconnect();

    void service(int timeoutMs,
                 const std::function<void()>& onConnect,
                 const std::function<void()>& onDisconnect,
                 const std::function<void(const ENetPacket*)>& onPacket);

    bool send(const std::vector<std::uint8_t>& data, bool reliable = false);

    ENetPeer* peerHandle() { return peer; }

private:
    ENetContext ctx;
    ENetHost* client{nullptr};
    ENetPeer* peer{nullptr};
};

} // namespace net
