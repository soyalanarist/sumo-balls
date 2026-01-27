#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

namespace net {

constexpr std::uint8_t PROTOCOL_VERSION = 1;

enum class MessageType : std::uint8_t {
    JoinRequest = 1,
    JoinAccept  = 2,
    Input       = 3,
    State       = 4,
    Ping        = 5,
    Pong        = 6
};

enum class ParseError {
    Success = 0,
    PacketTooShort,
    InvalidProtocolVersion,
    InvalidFieldValue,
    CorruptedData,
    UnknownMessageType,
    InvalidPlayerCount,
    InvalidMessageStructure
};

struct ParseResult {
    ParseError error{ParseError::Success};
    std::string context;
    std::size_t expectedBytes{0};
    std::size_t actualBytes{0};
    std::uint8_t expectedVersion{PROTOCOL_VERSION};
    std::uint8_t actualVersion{PROTOCOL_VERSION};
    std::string fieldName;

    std::string getErrorMessage() const;
};

struct JoinAccept {
    std::uint32_t playerId{0};
};

struct InputCommand {
    std::uint32_t playerId{0};
    float dirX{0.f};
    float dirY{0.f};
    std::uint32_t sequence{0};
    std::uint32_t timestampMs{0};
};

struct PlayerState {
    std::uint32_t playerId{0};
    float x{0.f};
    float y{0.f};
    float vx{0.f};
    float vy{0.f};
    std::uint8_t alive{1};
};

struct Ping {
    std::uint32_t timestampMs{0};
};

struct StateSnapshot {
    std::uint32_t tick{0};
    std::uint32_t serverTimeMs{0};
    float arenaRadius{0.f};
    std::vector<PlayerState> players;
};

inline bool deserializeState(const std::uint8_t* data, std::size_t len, StateSnapshot& out) {
    // Expect header already validated (type == State)
    constexpr std::size_t headerSize = 2;
    if (len < headerSize + sizeof(std::uint32_t) * 3 + sizeof(float)) return false;
    std::size_t offset = headerSize;
    std::memcpy(&out.tick, data + offset, sizeof(out.tick));
    offset += sizeof(out.tick);
    std::memcpy(&out.serverTimeMs, data + offset, sizeof(out.serverTimeMs));
    offset += sizeof(out.serverTimeMs);
    std::memcpy(&out.arenaRadius, data + offset, sizeof(out.arenaRadius));
    offset += sizeof(out.arenaRadius);
    std::uint32_t count = 0;
    std::memcpy(&count, data + offset, sizeof(count));
    offset += sizeof(count);
    if (len < offset + count * sizeof(PlayerState)) return false;
    out.players.resize(count);
    std::memcpy(out.players.data(), data + offset, count * sizeof(PlayerState));
    return true;
}

// Simple serialization helpers (little-endian, POD only)
inline void appendBytes(std::vector<std::uint8_t>& out, const void* data, std::size_t len) {
    const auto* bytes = static_cast<const std::uint8_t*>(data);
    out.insert(out.end(), bytes, bytes + len);
}

inline void appendMessageHeader(std::vector<std::uint8_t>& out, MessageType type) {
    out.push_back(PROTOCOL_VERSION);
    out.push_back(static_cast<std::uint8_t>(type));
}

inline std::vector<std::uint8_t> serializeInput(const InputCommand& cmd) {
    std::vector<std::uint8_t> out;
    out.reserve(2 + sizeof(InputCommand));
    appendMessageHeader(out, MessageType::Input);
    appendBytes(out, &cmd, sizeof(InputCommand));
    return out;
}

inline std::vector<std::uint8_t> serializeJoinAccept(const JoinAccept& msg) {
    std::vector<std::uint8_t> out;
    out.reserve(2 + sizeof(JoinAccept));
    appendMessageHeader(out, MessageType::JoinAccept);
    appendBytes(out, &msg, sizeof(JoinAccept));
    return out;
}

inline std::vector<std::uint8_t> serializePing(MessageType type, const Ping& msg) {
    std::vector<std::uint8_t> out;
    out.reserve(2 + sizeof(Ping));
    appendMessageHeader(out, type);
    appendBytes(out, &msg, sizeof(Ping));
    return out;
}

inline std::vector<std::uint8_t> serializeState(const StateSnapshot& snap) {
    std::vector<std::uint8_t> out;
    out.reserve(2 + sizeof(std::uint32_t) * 3 + sizeof(float) + snap.players.size() * sizeof(PlayerState));
    appendMessageHeader(out, MessageType::State);
    appendBytes(out, &snap.tick, sizeof(snap.tick));
    appendBytes(out, &snap.serverTimeMs, sizeof(snap.serverTimeMs));
    appendBytes(out, &snap.arenaRadius, sizeof(snap.arenaRadius));
    std::uint32_t count = static_cast<std::uint32_t>(snap.players.size());
    appendBytes(out, &count, sizeof(count));
    for (const auto& p : snap.players) {
        appendBytes(out, &p, sizeof(PlayerState));
    }
    return out;
}

// Lightweight parser for headers; full parsing can be done when handling events
inline bool parseHeader(const std::uint8_t* data, std::size_t len, MessageType& outType) {
    if (len < 2) return false;
    if (data[0] != PROTOCOL_VERSION) return false;
    outType = static_cast<MessageType>(data[1]);
    return true;
}

} // namespace net
