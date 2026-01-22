#include "NetServer.h"
#include <iostream>

namespace net {

NetServer::NetServer() = default;
NetServer::~NetServer() { stop(); }

bool NetServer::start(std::uint16_t port, std::size_t maxClients) {
    stop();

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = port;

    host = enet_host_create(&address, maxClients, 2, 0, 0);
    if (!host) {
        std::cerr << "[NetServer Error] Failed to create ENet server (port: " << port << ", maxClients: " << maxClients << ")" << std::endl;
        return false;
    }
    return true;
}

void NetServer::stop() {
    if (host) {
        enet_host_destroy(host);
        host = nullptr;
    }
}

void NetServer::service(int timeoutMs,
                        const std::function<void(ENetPeer*)>& onConnect,
                        const std::function<void(ENetPeer*)>& onDisconnect,
                        const std::function<void(ENetPeer*, const ENetPacket*)>& onPacket) {
    if (!host) return;

    ENetEvent event{};
    while (enet_host_service(host, &event, timeoutMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (onConnect) onConnect(event.peer);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                if (onDisconnect) onDisconnect(event.peer);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (onPacket) onPacket(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_NONE:
            default:
                break;
        }
    }
}

bool NetServer::broadcast(const std::vector<std::uint8_t>& data, bool reliable) {
    if (!host) return false;
    ENetPacket* packet = enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (!packet) {
        std::cerr << "[NetServer Warning] Failed to create broadcast packet (size: " << data.size() << " bytes)" << std::endl;
        return false;
    }
    enet_host_broadcast(host, reliable ? 1 : 0, packet);
    return true;
}

bool NetServer::sendTo(ENetPeer* peer, const std::vector<std::uint8_t>& data, bool reliable) {
    if (!peer) {
        std::cerr << "[NetServer Warning] Attempted to send to null peer" << std::endl;
        return false;
    }
    ENetPacket* packet = enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (!packet) {
        std::cerr << "[NetServer Warning] Failed to create packet for peer (size: " << data.size() << " bytes)" << std::endl;
        return false;
    }
    return enet_peer_send(peer, reliable ? 1 : 0, packet) == 0;
}

} // namespace net
