#pragma once
#include <map>
#include <string>

#include "ecs/IComponent.h"

namespace fresh
{
namespace rpg
{

/**
 * @brief Component for managing entity progression (XP and levels)
 */
class ProgressionComponent : public ecs::IComponent
{
public:
    ProgressionComponent();
    virtual ~ProgressionComponent() = default;

    COMPONENT_TYPE(ProgressionComponent)

    // Experience management
    void addExperience(float xp);
    float getExperience() const
    {
        return experience;
    }

    // Level management
    int getLevel() const
    {
        return level;
    }
    float getExperienceForNextLevel() const;
    float getProgressToNextLevel() const; // 0.0 to 1.0

    // Stats
    void setStat(const std::string& name, float value);
    float getStat(const std::string& name) const;

private:
    int level;
    float experience;
    float experienceInCurrentLevel;
    std::map<std::string, float> stats;

    void checkLevelUp();
};

} // namespace rpg
} // namespace fresh
