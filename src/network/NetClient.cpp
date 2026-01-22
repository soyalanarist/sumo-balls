#include "NetClient.h"
#include <iostream>

namespace net {

NetClient::NetClient() = default;
NetClient::~NetClient() { disconnect(); }

bool NetClient::connect(const std::string& host, std::uint16_t port) {
    disconnect();

    client = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!client) {
        std::cerr << "[NetClient Error] Failed to create ENet client host (target: " << host << ":" << port << ")" << std::endl;
        return false;
    }

    ENetAddress address{};
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer = enet_host_connect(client, &address, 2, 0);
    if (!peer) {
        std::cerr << "[NetClient Error] Failed to initiate connection to " << host << ":" << port << std::endl;
        disconnect();
        return false;
    }
    return true;
}

void NetClient::disconnect() {
    if (peer) {
        enet_peer_disconnect(peer, 0);
        ENetEvent event{};
        while (enet_host_service(client, &event, 100) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                enet_packet_destroy(event.packet);
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                break;
            }
        }
        peer = nullptr;
    }
    if (client) {
        enet_host_destroy(client);
        client = nullptr;
    }
}

void NetClient::service(int timeoutMs,
                        const std::function<void()>& onConnect,
                        const std::function<void()>& onDisconnect,
                        const std::function<void(const ENetPacket*)>& onPacket) {
    if (!client) return;

    ENetEvent event{};
    int eventCount = enet_host_service(client, &event, timeoutMs);
    
    while (eventCount > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                if (onConnect) onConnect();
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                if (onDisconnect) onDisconnect();
                peer = nullptr;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                if (onPacket) onPacket(event.packet);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_NONE:
            default:
                break;
        }
        eventCount = enet_host_service(client, &event, 0);
    }
}

bool NetClient::send(const std::vector<std::uint8_t>& data, bool reliable) {
    if (!peer) return false;
    ENetPacket* packet = enet_packet_create(data.data(), data.size(), reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    if (!packet) return false;
    return enet_peer_send(peer, reliable ? 1 : 0, packet) == 0;
}

} // namespace net
