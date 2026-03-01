#include "networking/NetHardening.h"

namespace fresh
{
namespace networking
{

void NetHardening::configure(const NetHardeningConfig& config)
{
    m_config = config;
}

const NetHardeningConfig& NetHardening::config() const
{
    return m_config;
}

void NetHardening::update(float deltaTimeMs)
{
    m_secondAccumulator += deltaTimeMs;
    m_totalTimeTracked += deltaTimeMs;
    if (m_secondAccumulator >= 1000.0f) {
        m_secondAccumulator -= 1000.0f;
        m_bytesSentThisSecond = 0.0f;
    }

    switch (m_stats.state) {
    case ConnectionState::Connecting:
        m_timeSinceConnectStart += deltaTimeMs;
        if (m_config.connectionTimeoutMs > 0
            && m_timeSinceConnectStart >= static_cast<float>(m_config.connectionTimeoutMs)) {
            handleTimeout();
        }
        break;

    case ConnectionState::Connected:
        if (m_config.heartbeatIntervalMs > 0) {
            m_timeSinceLastHeartbeat += deltaTimeMs;
            float threshold = static_cast<float>(m_config.heartbeatIntervalMs)
                              * static_cast<float>(m_config.heartbeatMissThreshold);
            if (m_timeSinceLastHeartbeat >= threshold) {
                handleTimeout();
            }
        }
        break;

    case ConnectionState::Reconnecting:
        m_timeSinceLastReconnect += deltaTimeMs;
        if (m_timeSinceLastReconnect >= static_cast<float>(m_config.reconnectDelayMs)) {
            attemptReconnect();
        }
        break;

    default:
        break;
    }
}

void NetHardening::connect()
{
    m_timeSinceConnectStart = 0.0f;
    m_reconnectAttempts = 0;
    setState(ConnectionState::Connecting);
}

void NetHardening::disconnect()
{
    setState(ConnectionState::Disconnected);
}

void NetHardening::recordBytesSent(uint32_t bytes)
{
    m_stats.bytesSent += bytes;
    m_bytesSentThisSecond += static_cast<float>(bytes);
    m_totalBytesTracked += static_cast<float>(bytes);
}

void NetHardening::recordBytesReceived(uint32_t bytes)
{
    m_stats.bytesReceived += bytes;
}

void NetHardening::recordPacketSent()
{
    m_stats.packetsSent++;
}

void NetHardening::recordPacketReceived()
{
    m_stats.packetsReceived++;

    // Receiving a packet while connecting means we're connected
    if (m_stats.state == ConnectionState::Connecting) {
        setState(ConnectionState::Connected);
    }
}

void NetHardening::recordPacketDropped()
{
    m_stats.packetsDropped++;
}

void NetHardening::recordRtt(float rttMs)
{
    if (rttMs > m_stats.peakRttMs) {
        m_stats.peakRttMs = rttMs;
    }
    // Exponential moving average
    if (m_stats.averageRttMs == 0.0f) {
        m_stats.averageRttMs = rttMs;
    } else {
        m_stats.averageRttMs = m_stats.averageRttMs * 0.9f + rttMs * 0.1f;
    }
}

void NetHardening::recordHeartbeat()
{
    m_timeSinceLastHeartbeat = 0.0f;
}

bool NetHardening::canSendBytes(uint32_t bytes) const
{
    if (m_config.maxBandwidthBytesPerSec == 0)
        return true;
    return (m_bytesSentThisSecond + static_cast<float>(bytes))
           <= static_cast<float>(m_config.maxBandwidthBytesPerSec);
}

bool NetHardening::isPacketSizeValid(uint32_t bytes) const
{
    return bytes <= m_config.maxPacketSize;
}

ConnectionState NetHardening::state() const
{
    return m_stats.state;
}

const ConnectionStats& NetHardening::stats() const
{
    return m_stats;
}

void NetHardening::setStateCallback(StateCallback cb)
{
    m_stateCallback = std::move(cb);
}

void NetHardening::resetStats()
{
    ConnectionState currentState = m_stats.state;
    m_stats = ConnectionStats();
    m_stats.state = currentState;
}

void NetHardening::setState(ConnectionState newState)
{
    ConnectionState oldState = m_stats.state;
    if (oldState == newState)
        return;
    m_stats.state = newState;
    if (m_stateCallback) {
        m_stateCallback(oldState, newState);
    }
}

void NetHardening::handleTimeout()
{
    if (m_config.maxReconnectAttempts > 0
        && m_reconnectAttempts < m_config.maxReconnectAttempts) {
        m_timeSinceLastReconnect = 0.0f;
        setState(ConnectionState::Reconnecting);
    } else {
        setState(ConnectionState::TimedOut);
    }
}

void NetHardening::attemptReconnect()
{
    m_reconnectAttempts++;
    m_stats.reconnectCount = m_reconnectAttempts;
    m_timeSinceLastReconnect = 0.0f;

    if (m_reconnectAttempts >= m_config.maxReconnectAttempts) {
        setState(ConnectionState::TimedOut);
    } else {
        // Transition back to connecting to retry
        m_timeSinceConnectStart = 0.0f;
        setState(ConnectionState::Connecting);
    }
}

void NetHardening::setPacketLossSimulation(const PacketLossSimConfig& simConfig)
{
    m_lossSimConfig = simConfig;
}

const PacketLossSimConfig& NetHardening::packetLossSimulation() const
{
    return m_lossSimConfig;
}

bool NetHardening::shouldDropPacket() const
{
    if (!m_lossSimConfig.enabled || m_lossSimConfig.lossPercent <= 0.0f)
        return false;
    // Deterministic drop based on counter
    m_lossCounter++;
    uint32_t threshold = static_cast<uint32_t>(100.0f / m_lossSimConfig.lossPercent);
    if (threshold == 0)
        threshold = 1;
    return (m_lossCounter % threshold) == 0;
}

ConnectionQuality NetHardening::getConnectionQuality() const
{
    float rtt = m_stats.averageRttMs;
    float loss = packetLossPercent();

    if (rtt < 30.0f && loss < 1.0f)
        return ConnectionQuality::Excellent;
    if (rtt < 80.0f && loss < 3.0f)
        return ConnectionQuality::Good;
    if (rtt < 150.0f && loss < 8.0f)
        return ConnectionQuality::Fair;
    if (rtt < 300.0f && loss < 15.0f)
        return ConnectionQuality::Poor;
    return ConnectionQuality::Critical;
}

std::string NetHardening::connectionQualityString() const
{
    switch (getConnectionQuality()) {
    case ConnectionQuality::Excellent:
        return "Excellent";
    case ConnectionQuality::Good:
        return "Good";
    case ConnectionQuality::Fair:
        return "Fair";
    case ConnectionQuality::Poor:
        return "Poor";
    case ConnectionQuality::Critical:
        return "Critical";
    }
    return "Unknown";
}

uint32_t NetHardening::getSimulatedLatencyMs() const
{
    if (!m_lossSimConfig.enabled)
        return 0;
    float lat = m_lossSimConfig.latencyMs;
    if (m_lossSimConfig.jitterMs > 0.0f) {
        // Deterministic jitter based on counter
        float jitter =
            m_lossSimConfig.jitterMs * (static_cast<float>(m_lossCounter % 100) / 50.0f - 1.0f);
        lat += jitter;
    }
    return static_cast<uint32_t>(lat > 0.0f ? lat : 0.0f);
}

float NetHardening::packetLossPercent() const
{
    uint32_t total = m_stats.packetsSent + m_stats.packetsReceived;
    if (total == 0)
        return 0.0f;
    return static_cast<float>(m_stats.packetsDropped) / static_cast<float>(total) * 100.0f;
}

float NetHardening::averageBandwidthBytesPerSec() const
{
    if (m_totalTimeTracked <= 0.0f)
        return 0.0f;
    return m_totalBytesTracked / (m_totalTimeTracked / 1000.0f);
}

} // namespace networking
} // namespace fresh
