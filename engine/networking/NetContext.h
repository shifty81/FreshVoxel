#pragma once
// ============================================================
// FreshVoxel NetContext — Packet-Based Networking Layer
// ============================================================
//
// Adapted from AtlasForge engine. Provides a high-level
// packet-based networking context with lockstep/rollback
// support, P2P modes, input recording, and snapshot
// management for deterministic multiplayer.
//
// Integrates with the existing GameServer/ClientConnection
// infrastructure for actual socket transport while adding
// the missing game-state synchronization layer.

#include <cstdint>
#include <functional>
#include <queue>
#include <string>
#include <vector>

namespace fresh
{
namespace ecs
{
class EntityManager;
}
} // namespace fresh

namespace fresh
{
namespace networking
{

class NetHardening;

/// Version of the network packet schema.
/// Increment when the Packet struct layout changes.
constexpr uint32_t NET_PACKET_SCHEMA_VERSION = 1;

enum class NetMode {
    Standalone,
    Client,
    Server,
    P2P_Host,
    P2P_Peer
};

struct Packet {
    uint16_t type = 0;
    uint16_t size = 0;
    uint32_t tick = 0;
    uint32_t checksum = 0;
    std::vector<uint8_t> payload;
};

struct NetPeer {
    uint32_t id = 0;
    float rtt = 0.0f;
    bool connected = false;
};

struct InputFrame {
    uint32_t tick = 0;
    uint32_t playerID = 0;
    float moveX = 0.0f;
    float moveY = 0.0f;
};

struct WorldSnapshot {
    uint32_t tick = 0;
    std::vector<uint8_t> ecsState;
};

struct QueuedPacket {
    uint32_t destPeerID = 0; // 0 = broadcast
    Packet packet;
};

/**
 * @brief High-level networking context for game-state synchronization
 *
 * Provides lockstep/rollback networking, input recording,
 * snapshot management, and P2P support on top of the existing
 * GameServer/ClientConnection transport layer.
 */
class NetContext
{
public:
    void init(NetMode mode);
    void shutdown();

    void poll();
    void send(uint32_t peerID, const Packet& pkt);
    void broadcast(const Packet& pkt);
    void flush();

    NetMode mode() const;
    const std::vector<NetPeer>& peers() const;
    bool isAuthority() const;

    // Peer management
    uint32_t addPeer();
    void removePeer(uint32_t peerID);

    // Receive incoming packets (from local queue after poll)
    bool receive(Packet& outPkt);

    // ECS world binding (required for snapshot/rollback)
    void setEntityManager(ecs::EntityManager* entityManager);

    // Input frame recording for replay
    void recordInput(const InputFrame& frame);
    const std::vector<InputFrame>& recordedInputs() const;

    // Set callback for applying input frames during replay
    void setInputApplyCallback(std::function<void(const InputFrame&)> cb);

    // Lockstep / Rollback
    void saveSnapshot(uint32_t tick);
    void rollbackTo(uint32_t tick);
    void replayFrom(uint32_t tick);

    const std::vector<WorldSnapshot>& snapshots() const;

    // Save tick broadcasting
    void broadcastSaveTick(uint32_t tick, uint64_t stateHash);
    uint32_t lastSaveTick() const;
    uint64_t lastSaveHash() const;

    /// Returns the packet schema version this context was built with.
    static uint32_t packetSchemaVersion() { return NET_PACKET_SCHEMA_VERSION; }

    /// Set a NetHardening instance for bandwidth/loss enforcement.
    void setHardening(NetHardening* hardening);

    /// Number of packets dropped due to hardening (bandwidth/loss).
    uint32_t droppedSendCount() const;

    /// Number of packets dropped due to invalid checksum on receive.
    uint32_t invalidChecksumCount() const;

    /// Compute a CRC32 checksum over data.
    static uint32_t computeChecksum(const uint8_t* data, size_t size);

    /// Validate a packet's checksum field against its payload.
    static bool validateChecksum(const Packet& pkt);

private:
    NetMode m_mode = NetMode::Standalone;
    std::vector<NetPeer> m_peers;
    std::vector<WorldSnapshot> m_snapshots;
    std::vector<InputFrame> m_inputHistory;
    uint32_t m_nextPeerID = 1;

    // Bound ECS entity manager for serialization
    ecs::EntityManager* m_entityManager = nullptr;

    // Local packet queues for testability
    std::queue<QueuedPacket> m_outgoing;
    std::queue<Packet> m_incoming;

    // Optional callback for applying input frames during replay
    std::function<void(const InputFrame&)> m_inputApplyCallback;

    // Save tick broadcasting state
    uint32_t m_lastSaveTick = 0;
    uint64_t m_lastSaveHash = 0;

    // Hardening integration
    NetHardening* m_hardening = nullptr;
    uint32_t m_droppedSendCount = 0;
    uint32_t m_invalidChecksumCount = 0;
};

} // namespace networking
} // namespace fresh
