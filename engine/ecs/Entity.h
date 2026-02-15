#pragma once
#include <cstdint>

namespace fresh
{
namespace ecs
{

/**
 * @brief Represents a game object with a unique identifier
 *
 * Entity is a lightweight identifier used in the Entity-Component System.
 * The actual data and behavior are stored in Components and Systems.
 */
class Entity
{
public:
    using ID = uint64_t;
    static constexpr ID INVALID_ID = 0;

    Entity() : id(INVALID_ID) {}
    explicit Entity(ID entityId) : id(entityId) {}

    ID getId() const
    {
        return id;
    }
    bool isValid() const
    {
        return id != INVALID_ID;
    }

    bool operator==(const Entity& other) const
    {
        return id == other.id;
    }
    bool operator!=(const Entity& other) const
    {
        return id != other.id;
    }
    bool operator<(const Entity& other) const
    {
        return id < other.id;
    }

private:
    ID id;
};

} // namespace ecs
} // namespace fresh
