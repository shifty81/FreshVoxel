#pragma once
#include <map>
#include <string>

#include "ecs/IComponent.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Represents faction relationships
 */
enum class FactionRelation { Hostile, Unfriendly, Neutral, Friendly, Allied };

/**
 * @brief Component for handling faction relations
 */
class FactionComponent : public ecs::IComponent
{
public:
    FactionComponent();
    explicit FactionComponent(const std::string& factionName);
    virtual ~FactionComponent() = default;

    COMPONENT_TYPE(FactionComponent)

    // Faction identity
    const std::string& getFactionName() const
    {
        return factionName;
    }
    void setFactionName(const std::string& name)
    {
        factionName = name;
    }

    // Reputation management
    void setReputation(const std::string& faction, float rep);
    float getReputation(const std::string& faction) const;
    void modifyReputation(const std::string& faction, float delta);

    // Relation queries
    FactionRelation getRelation(const std::string& faction) const;
    bool isHostile(const std::string& faction) const;
    bool isFriendly(const std::string& faction) const;

private:
    std::string factionName;
    std::map<std::string, float> reputations; // faction -> reputation (-100 to +100)

    FactionRelation reputationToRelation(float reputation) const;
};

} // namespace rpg
} // namespace fresh
