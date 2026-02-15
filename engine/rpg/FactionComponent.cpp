#include "rpg/FactionComponent.h"

namespace fresh
{
namespace rpg
{

FactionComponent::FactionComponent() : factionName("Independent") {}

FactionComponent::FactionComponent(const std::string& factionName) : factionName(factionName) {}

void FactionComponent::setReputation(const std::string& faction, float rep)
{
    // Clamp reputation to [-100, +100]
    reputations[faction] = std::max(-100.0f, std::min(100.0f, rep));
}

float FactionComponent::getReputation(const std::string& faction) const
{
    auto it = reputations.find(faction);
    return (it != reputations.end()) ? it->second : 0.0f;
}

void FactionComponent::modifyReputation(const std::string& faction, float delta)
{
    float current = getReputation(faction);
    setReputation(faction, current + delta);
}

FactionRelation FactionComponent::getRelation(const std::string& faction) const
{
    return reputationToRelation(getReputation(faction));
}

bool FactionComponent::isHostile(const std::string& faction) const
{
    return getRelation(faction) == FactionRelation::Hostile;
}

bool FactionComponent::isFriendly(const std::string& faction) const
{
    FactionRelation rel = getRelation(faction);
    return rel == FactionRelation::Friendly || rel == FactionRelation::Allied;
}

FactionRelation FactionComponent::reputationToRelation(float reputation) const
{
    if (reputation <= -50.0f)
        return FactionRelation::Hostile;
    if (reputation <= -10.0f)
        return FactionRelation::Unfriendly;
    if (reputation <= 10.0f)
        return FactionRelation::Neutral;
    if (reputation <= 50.0f)
        return FactionRelation::Friendly;
    return FactionRelation::Allied;
}

} // namespace rpg
} // namespace fresh
