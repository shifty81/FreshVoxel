#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace fresh
{
namespace networking
{

/**
 * @brief Message types for network communication
 */
enum class MessageType : uint16_t {
    Connect = 1,
    Disconnect = 2,
    PlayerJoin = 3,
    PlayerLeave = 4,
    EntityUpdate = 5,
    SectorChange = 6,
    ChatMessage = 7,
    ActionCommand = 8,
    InventoryUpdate = 9,
    CombatEvent = 10
};

/**
 * @brief Network message structure
 */
class NetworkMessage
{
public:
    NetworkMessage();
    NetworkMessage(MessageType type);

    // Getters
    MessageType getType() const
    {
        return type;
    }
    uint32_t getSize() const
    {
        return static_cast<uint32_t>(data.size());
    }
    const std::vector<uint8_t>& getData() const
    {
        return data;
    }

    // Serialization
    void writeByte(uint8_t value);
    void writeInt16(int16_t value);
    void writeInt32(int32_t value);
    void writeFloat(float value);
    void writeString(const std::string& value);
    void writeBytes(const uint8_t* bytes, size_t length);

    // Deserialization
    uint8_t readByte();
    int16_t readInt16();
    int32_t readInt32();
    float readFloat();
    std::string readString();
    void readBytes(uint8_t* buffer, size_t length);

    // Reset read position
    void resetReadPosition()
    {
        readPosition = 0;
    }

    // Serialize/deserialize entire message for transmission
    std::vector<uint8_t> serialize() const;
    static std::unique_ptr<NetworkMessage> deserialize(const uint8_t* buffer, size_t length);

private:
    MessageType type;
    std::vector<uint8_t> data;
    size_t readPosition;
};

} // namespace networking
} // namespace fresh
