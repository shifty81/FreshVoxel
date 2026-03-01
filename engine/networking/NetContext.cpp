#include "networking/NetContext.h"

#include "networking/NetHardening.h"

#include <algorithm>
#include <cstring>

namespace fresh
{
namespace networking
{

void NetContext::init(NetMode mode)
{
    m_mode = mode;
    m_peers.clear();
    m_snapshots.clear();
    m_inputHistory.clear();
    m_nextPeerID = 1;
    m_hardening = nullptr;
    m_droppedSendCount = 0;
    m_invalidChecksumCount = 0;
    while (!m_outgoing.empty())
        m_outgoing.pop();
    while (!m_incoming.empty())
        m_incoming.pop();
}

void NetContext::shutdown()
{
    m_peers.clear();
    m_snapshots.clear();
    m_inputHistory.clear();
    while (!m_outgoing.empty())
        m_outgoing.pop();
    while (!m_incoming.empty())
        m_incoming.pop();
    m_mode = NetMode::Standalone;
}

void NetContext::poll()
{
    // In local/loopback mode, move outgoing packets to incoming.
    // This enables testing without real sockets.
    while (!m_outgoing.empty()) {
        m_incoming.push(m_outgoing.front().packet);
        m_outgoing.pop();
    }
}

void NetContext::send(uint32_t peerID, const Packet& pkt)
{
    if (m_hardening) {
        if (!m_hardening->canSendBytes(static_cast<uint32_t>(pkt.payload.size()))) {
            m_droppedSendCount++;
            return;
        }
        if (m_hardening->shouldDropPacket()) {
            m_droppedSendCount++;
            return;
        }
    }

    QueuedPacket qp;
    qp.destPeerID = peerID;
    qp.packet = pkt;
    qp.packet.checksum = computeChecksum(pkt.payload.data(), pkt.payload.size());
    m_outgoing.push(qp);

    if (m_hardening) {
        m_hardening->recordBytesSent(static_cast<uint32_t>(pkt.payload.size()));
        m_hardening->recordPacketSent();
    }
}

void NetContext::broadcast(const Packet& pkt)
{
    if (m_hardening) {
        if (!m_hardening->canSendBytes(static_cast<uint32_t>(pkt.payload.size()))) {
            m_droppedSendCount++;
            return;
        }
        if (m_hardening->shouldDropPacket()) {
            m_droppedSendCount++;
            return;
        }
    }

    QueuedPacket qp;
    qp.destPeerID = 0; // 0 = broadcast
    qp.packet = pkt;
    qp.packet.checksum = computeChecksum(pkt.payload.data(), pkt.payload.size());
    m_outgoing.push(qp);

    if (m_hardening) {
        m_hardening->recordBytesSent(static_cast<uint32_t>(pkt.payload.size()));
        m_hardening->recordPacketSent();
    }
}

void NetContext::flush()
{
    // In a real implementation, this would push buffered data to sockets.
    // For local mode, poll() already handles the transfer.
}

NetMode NetContext::mode() const
{
    return m_mode;
}

const std::vector<NetPeer>& NetContext::peers() const
{
    return m_peers;
}

bool NetContext::isAuthority() const
{
    return m_mode == NetMode::Server || m_mode == NetMode::P2P_Host;
}

uint32_t NetContext::addPeer()
{
    NetPeer peer;
    peer.id = m_nextPeerID++;
    peer.rtt = 0.0f;
    peer.connected = true;
    m_peers.push_back(peer);
    return peer.id;
}

void NetContext::removePeer(uint32_t peerID)
{
    m_peers.erase(std::remove_if(m_peers.begin(), m_peers.end(),
                                 [peerID](const NetPeer& p) { return p.id == peerID; }),
                  m_peers.end());
}

bool NetContext::receive(Packet& outPkt)
{
    if (m_incoming.empty())
        return false;
    outPkt = m_incoming.front();
    m_incoming.pop();
    if (!validateChecksum(outPkt)) {
        m_invalidChecksumCount++;
        return false;
    }
    return true;
}

void NetContext::setEntityManager(ecs::EntityManager* entityManager)
{
    m_entityManager = entityManager;
}

void NetContext::recordInput(const InputFrame& frame)
{
    m_inputHistory.push_back(frame);
}

const std::vector<InputFrame>& NetContext::recordedInputs() const
{
    return m_inputHistory;
}

void NetContext::setInputApplyCallback(std::function<void(const InputFrame&)> cb)
{
    m_inputApplyCallback = std::move(cb);
}

void NetContext::saveSnapshot(uint32_t tick)
{
    WorldSnapshot snap;
    snap.tick = tick;
    // ECS serialization would go here when EntityManager supports it
    m_snapshots.push_back(std::move(snap));
}

void NetContext::rollbackTo(uint32_t tick)
{
    // Find the snapshot for the requested tick
    const WorldSnapshot* target = nullptr;
    for (const auto& snap : m_snapshots) {
        if (snap.tick == tick) {
            target = &snap;
            break;
        }
    }

    if (target && m_entityManager && !target->ecsState.empty()) {
        // ECS deserialization would go here when EntityManager supports it
    }

    // Remove snapshots after the rollback tick
    m_snapshots.erase(
        std::remove_if(m_snapshots.begin(), m_snapshots.end(),
                       [tick](const WorldSnapshot& s) { return s.tick > tick; }),
        m_snapshots.end());
}

void NetContext::replayFrom(uint32_t tick)
{
    if (!m_entityManager)
        return;

    for (const auto& frame : m_inputHistory) {
        if (frame.tick >= tick) {
            if (m_inputApplyCallback) {
                m_inputApplyCallback(frame);
            }
        }
    }
}

const std::vector<WorldSnapshot>& NetContext::snapshots() const
{
    return m_snapshots;
}

void NetContext::broadcastSaveTick(uint32_t tick, uint64_t stateHash)
{
    m_lastSaveTick = tick;
    m_lastSaveHash = stateHash;

    Packet pkt;
    pkt.type = 0xFFFF; // Save-tick sentinel
    pkt.tick = tick;
    pkt.payload.resize(8);
    std::memcpy(pkt.payload.data(), &stateHash, 8);
    broadcast(pkt);
}

uint32_t NetContext::lastSaveTick() const
{
    return m_lastSaveTick;
}

uint64_t NetContext::lastSaveHash() const
{
    return m_lastSaveHash;
}

void NetContext::setHardening(NetHardening* hardening)
{
    m_hardening = hardening;
}

uint32_t NetContext::droppedSendCount() const
{
    return m_droppedSendCount;
}

uint32_t NetContext::invalidChecksumCount() const
{
    return m_invalidChecksumCount;
}

uint32_t NetContext::computeChecksum(const uint8_t* data, size_t size)
{
    // CRC32 (IEEE 802.3 polynomial)
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

bool NetContext::validateChecksum(const Packet& pkt)
{
    if (pkt.payload.empty() && pkt.checksum == 0)
        return true;
    return pkt.checksum == computeChecksum(pkt.payload.data(), pkt.payload.size());
}

} // namespace networking
} // namespace fresh
