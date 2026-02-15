#include "rpg/ProgressionComponent.h"

#include <cmath>

namespace fresh
{
namespace rpg
{

ProgressionComponent::ProgressionComponent()
    : level(1), experience(0.0f), experienceInCurrentLevel(0.0f)
{
}

void ProgressionComponent::addExperience(float xp)
{
    if (xp <= 0.0f)
        return;

    experience += xp;
    experienceInCurrentLevel += xp;
    checkLevelUp();
}

float ProgressionComponent::getExperienceForNextLevel() const
{
    // Experience required grows exponentially with level
    return 100.0f * static_cast<float>(std::pow(1.5f, level - 1));
}

float ProgressionComponent::getProgressToNextLevel() const
{
    float required = getExperienceForNextLevel();
    return (required > 0.0f) ? (experienceInCurrentLevel / required) : 1.0f;
}

void ProgressionComponent::setStat(const std::string& name, float value)
{
    stats[name] = value;
}

float ProgressionComponent::getStat(const std::string& name) const
{
    auto it = stats.find(name);
    return (it != stats.end()) ? it->second : 0.0f;
}

void ProgressionComponent::checkLevelUp()
{
    float required = getExperienceForNextLevel();

    while (experienceInCurrentLevel >= required) {
        level++;
        experienceInCurrentLevel -= required;
        required = getExperienceForNextLevel();

        // Grant stat bonuses on level up
        setStat("maxHealth", getStat("maxHealth") + 10.0f);
        setStat("damage", getStat("damage") + 2.0f);
    }
}

} // namespace rpg
} // namespace fresh
