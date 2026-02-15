#pragma once

#include <string>

namespace fresh
{

class VoxelWorld;

/**
 * @brief Serialization system for saving/loading worlds
 */
class WorldSerializer
{
public:
    WorldSerializer();
    ~WorldSerializer();

    /**
     * @brief Save world to file
     * @param world World to save
     * @param filename Output filename
     * @return true if successful
     */
    bool saveWorld(const VoxelWorld* world, const std::string& filename);

    /**
     * @brief Load world from file
     * @param world World to load into
     * @param filename Input filename
     * @return true if successful
     */
    bool loadWorld(VoxelWorld* world, const std::string& filename);
};

} // namespace fresh
