#pragma once
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "character/VoxelCharacter.h"
#include "voxel/MeshGenerator.h"

namespace fresh
{

// Forward declarations
class RenderContext;

/**
 * @brief Mesh data for a voxel character
 */
struct CharacterMesh {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexCount;
    uint32_t indexCount;

    // Render resources (platform-specific)
    void* vertexBuffer;
    void* indexBuffer;

    CharacterMesh() : vertexCount(0), indexCount(0), vertexBuffer(nullptr), indexBuffer(nullptr) {}
};

/**
 * @brief Renders voxel characters as meshes
 *
 * Converts character voxels into renderable geometry and manages
 * rendering resources.
 */
class CharacterRenderer
{
public:
    CharacterRenderer();
    ~CharacterRenderer();

    /**
     * @brief Initialize the renderer with render context
     * @param ctx Rendering context for creating GPU resources
     */
    void initialize(RenderContext* ctx);

    /**
     * @brief Shutdown and cleanup resources
     */
    void shutdown();

    /**
     * @brief Generate mesh for a character
     * @param character Character to generate mesh for
     * @return Character mesh data
     */
    CharacterMesh generateMesh(const VoxelCharacter& character);

    /**
     * @brief Update an existing character mesh
     * @param character Character with updated voxels
     * @param mesh Mesh to update
     */
    void updateMesh(const VoxelCharacter& character, CharacterMesh& mesh);

    /**
     * @brief Render a character mesh
     * @param mesh Character mesh to render
     * @param worldTransform World transformation matrix
     */
    void render(const CharacterMesh& mesh, const glm::mat4& worldTransform);

    /**
     * @brief Release mesh resources
     * @param mesh Mesh to clean up
     */
    void releaseMesh(CharacterMesh& mesh);

    /**
     * @brief Enable/disable wireframe rendering
     */
    void setWireframe(bool enabled)
    {
        this->wireframe = enabled;
    }
    bool isWireframe() const
    {
        return wireframe;
    }

private:
    /**
     * @brief Generate vertices from character voxels
     * @param character Character to process
     * @param outVertices Output vertex data
     * @param outIndices Output index data
     */
    void generateVertices(const VoxelCharacter& character, std::vector<float>& outVertices,
                          std::vector<uint32_t>& outIndices);

    /**
     * @brief Add a voxel cube to the mesh
     * @param position Voxel position
     * @param color Voxel color
     * @param vertices Output vertices
     * @param indices Output indices
     */
    void addVoxelCube(const glm::ivec3& position, const glm::vec3& color,
                      std::vector<float>& vertices, std::vector<uint32_t>& indices);

    /**
     * @brief Create GPU buffers for mesh
     * @param mesh Mesh to create buffers for
     */
    void createBuffers(CharacterMesh& mesh);

    RenderContext* renderContext;
    bool wireframe;
    bool initialized;
};

/**
 * @brief Manager for multiple character instances
 *
 * Handles rendering of multiple characters efficiently with instancing
 * and batching.
 */
class CharacterRenderManager
{
public:
    CharacterRenderManager();
    ~CharacterRenderManager();

    /**
     * @brief Initialize the manager
     * @param charRenderer Character renderer to use
     */
    void initialize(CharacterRenderer* charRenderer);

    /**
     * @brief Register a character for rendering
     * @param character Character to register
     * @return Character instance ID
     */
    int registerCharacter(VoxelCharacter* character);

    /**
     * @brief Unregister a character
     * @param instanceId Character instance ID
     */
    void unregisterCharacter(int instanceId);

    /**
     * @brief Update character mesh (call when character voxels change)
     * @param instanceId Character instance ID
     */
    void updateCharacter(int instanceId);

    /**
     * @brief Render all registered characters
     * @param viewMatrix View matrix
     * @param projectionMatrix Projection matrix
     */
    void renderAll(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);

    /**
     * @brief Clear all registered characters
     */
    void clear();

private:
    struct CharacterInstance {
        VoxelCharacter* character;
        CharacterMesh mesh;
        bool needsUpdate;
    };

    CharacterRenderer* renderer;
    std::vector<CharacterInstance> instances;
    int nextInstanceId;
};

} // namespace fresh
