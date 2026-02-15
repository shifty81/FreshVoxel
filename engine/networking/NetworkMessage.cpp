#include "networking/NetworkMessage.h"

#include <cstring>

namespace fresh
{
namespace networking
{

NetworkMessage::NetworkMessage() : type(MessageType::Connect), readPosition(0) {}

NetworkMessage::NetworkMessage(MessageType type) : type(type), readPosition(0) {}

void NetworkMessage::writeByte(uint8_t value)
{
    data.push_back(value);
}

void NetworkMessage::writeInt16(int16_t value)
{
    data.push_back(static_cast<uint8_t>(value & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void NetworkMessage::writeInt32(int32_t value)
{
    data.push_back(static_cast<uint8_t>(value & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    data.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void NetworkMessage::writeFloat(float value)
{
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(float));
    writeInt32(static_cast<int32_t>(intValue));
}

void NetworkMessage::writeString(const std::string& value)
{
    writeInt16(static_cast<int16_t>(value.length()));
    for (char c : value) {
        writeByte(static_cast<uint8_t>(c));
    }
}

void NetworkMessage::writeBytes(const uint8_t* bytes, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        data.push_back(bytes[i]);
    }
}

uint8_t NetworkMessage::readByte()
{
    if (readPosition >= data.size())
        return 0;
    return data[readPosition++];
}

int16_t NetworkMessage::readInt16()
{
    int16_t value = 0;
    value |= static_cast<int16_t>(readByte());
    value |= static_cast<int16_t>(readByte()) << 8;
    return value;
}

int32_t NetworkMessage::readInt32()
{
    int32_t value = 0;
    value |= static_cast<int32_t>(readByte());
    value |= static_cast<int32_t>(readByte()) << 8;
    value |= static_cast<int32_t>(readByte()) << 16;
    value |= static_cast<int32_t>(readByte()) << 24;
    return value;
}

float NetworkMessage::readFloat()
{
    int32_t intValue = readInt32();
    float floatValue;
    std::memcpy(&floatValue, &intValue, sizeof(float));
    return floatValue;
}

std::string NetworkMessage::readString()
{
    int16_t length = readInt16();
    std::string result;
    result.reserve(length);
    for (int16_t i = 0; i < length; ++i) {
        result += static_cast<char>(readByte());
    }
    return result;
}

void NetworkMessage::readBytes(uint8_t* buffer, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        buffer[i] = readByte();
    }
}

std::vector<uint8_t> NetworkMessage::serialize() const
{
    std::vector<uint8_t> result;

    // Write message type (2 bytes)
    uint16_t typeValue = static_cast<uint16_t>(type);
    result.push_back(static_cast<uint8_t>(typeValue & 0xFF));
    result.push_back(static_cast<uint8_t>((typeValue >> 8) & 0xFF));

    // Write data size (4 bytes)
    uint32_t dataSize = static_cast<uint32_t>(data.size());
    result.push_back(static_cast<uint8_t>(dataSize & 0xFF));
    result.push_back(static_cast<uint8_t>((dataSize >> 8) & 0xFF));
    result.push_back(static_cast<uint8_t>((dataSize >> 16) & 0xFF));
    result.push_back(static_cast<uint8_t>((dataSize >> 24) & 0xFF));

    // Write data
    result.insert(result.end(), data.begin(), data.end());

    return result;
}

std::unique_ptr<NetworkMessage> NetworkMessage::deserialize(const uint8_t* buffer, size_t length)
{
    if (length < 6) {
        return nullptr;
    }

    // Read message type
    uint16_t typeValue = buffer[0] | (static_cast<uint16_t>(buffer[1]) << 8);
    MessageType type = static_cast<MessageType>(typeValue);

    // Read data size
    uint32_t dataSize = buffer[2] | (static_cast<uint32_t>(buffer[3]) << 8) |
                        (static_cast<uint32_t>(buffer[4]) << 16) |
                        (static_cast<uint32_t>(buffer[5]) << 24);

    if (length < 6 + dataSize) {
        return nullptr;
    }

    // Create message
    auto message = std::make_unique<NetworkMessage>(type);
    message->writeBytes(buffer + 6, dataSize);
    message->resetReadPosition();

    return message;
}

} // namespace networking
} // namespace fresh
