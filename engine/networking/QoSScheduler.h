#pragma once
// ============================================================
// FreshVoxel QoS Packet Scheduler — Priority-Based Ordering
// ============================================================
//
// Adapted from AtlasForge engine. Provides quality-of-service
// scheduling for outbound network packets. Each packet is
// assigned a priority and optional channel. The scheduler
// orders packets by priority (highest first) and enforces
// per-second bandwidth budgets with congestion detection.

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace fresh
{
namespace networking
{

/// Priority levels for network packets (higher = more important).
enum class PacketPriority : uint8_t {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};

/// Congestion state of the network link.
enum class CongestionState {
    Clear,       ///< No congestion detected
    Approaching, ///< Usage nearing budget
    Congested    ///< Budget exceeded — low-priority packets deferred
};

/// A queued outbound packet.
struct QoSPacket {
    uint32_t id = 0;
    PacketPriority priority = PacketPriority::Normal;
    uint32_t sizeBytes = 0;
    uint8_t channel = 0;      ///< Logical channel (0 = default)
    uint64_t enqueueTick = 0; ///< Tick when the packet was enqueued
};

/// Configuration for the QoS scheduler.
struct QoSConfig {
    /// Maximum bytes per second budget. 0 = unlimited.
    uint32_t bandwidthBudgetBytesPerSec = 0;

    /// Fraction of budget at which congestion warning fires (0.0-1.0).
    float congestionThreshold = 0.8f;

    /// Maximum number of queued packets. Oldest low-priority packets
    /// are dropped when the queue exceeds this limit.
    uint32_t maxQueueSize = 256;
};

/**
 * @brief Priority-based packet scheduler with congestion detection
 *
 * Provides quality-of-service scheduling for outbound network
 * packets with bandwidth budgets and priority ordering.
 */
class QoSScheduler
{
public:
    void configure(const QoSConfig& config);
    const QoSConfig& config() const;

    /// Enqueue a packet for scheduling.
    /// Returns false if the queue is full and the packet was dropped.
    bool enqueue(const QoSPacket& packet);

    /// Dequeue the next packet to send according to priority ordering.
    /// Returns false if the queue is empty.
    bool dequeue(QoSPacket& out);

    /// Number of packets currently queued.
    size_t queueSize() const;

    /// True if the queue contains any packets.
    bool hasPending() const;

    /// Drain all packets from the queue in priority order.
    std::vector<QoSPacket> drainAll();

    // --- Bandwidth & Congestion ---

    /// Record that bytes were actually sent this tick.
    void recordBytesSent(uint32_t bytes);

    /// Call once per second (or per tick with deltaMs) to advance
    /// the bandwidth tracking window.
    void updateWindow(float deltaMs);

    /// Bytes sent in the current tracking window.
    uint32_t bytesSentThisWindow() const;

    /// Current congestion state.
    CongestionState congestion() const;

    /// Total packets dropped due to queue overflow.
    uint32_t droppedCount() const;

    /// Reset all statistics and clear the queue.
    void reset();

private:
    void sortQueue();
    void enforceSizeLimit();

    QoSConfig m_config;
    std::vector<QoSPacket> m_queue;
    uint32_t m_nextId = 1;
    uint32_t m_bytesSentThisWindow = 0;
    float m_windowAccumulator = 0.0f;
    uint32_t m_droppedCount = 0;
};

} // namespace networking
} // namespace fresh
