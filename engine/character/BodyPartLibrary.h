#pragma once
#include <map>
#include <memory>

#include "character/VoxelCharacter.h"

namespace fresh
{

/**
 * @brief Library of pre-defined body part templates for modular character assembly
 *
 * Provides a collection of head, torso, and limb variations that can be
 * combined to create diverse character designs.
 */
class BodyPartLibrary
{
public:
    BodyPartLibrary();
    ~BodyPartLibrary();

    /**
     * @brief Get a body part template by type and variation index
     * @param type Body part type
     * @param variationIndex Index of variation (0-based)
     * @return Body part template, or nullptr if not found
     */
    const BodyPartTemplate* getTemplate(BodyPartType type, int variationIndex) const;

    /**
     * @brief Get number of variations available for a body part type
     * @param type Body part type
     * @return Number of variations
     */
    int getVariationCount(BodyPartType type) const;

    /**
     * @brief Create a random character from available templates
     * @param seed Random seed
     * @return Vector of body part templates
     */
    std::vector<BodyPartTemplate> createRandomCharacter(uint32_t seed) const;

    /**
     * @brief Initialize the library with default templates
     */
    void initialize();

private:
    /**
     * @brief Generate head variations
     */
    void generateHeadVariations();

    /**
     * @brief Generate torso variations
     */
    void generateTorsoVariations();

    /**
     * @brief Generate limb variations
     */
    void generateLimbVariations();

    // Storage for templates: type -> variation index -> template
    std::map<BodyPartType, std::vector<BodyPartTemplate>> templates;
};

} // namespace fresh
