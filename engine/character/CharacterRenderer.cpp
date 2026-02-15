#include "character/CharacterRenderer.h"

#include "renderer/RenderContext.h"

namespace fresh
{

// CharacterRenderer implementation

CharacterRenderer::CharacterRenderer()
    : renderContext(nullptr), wireframe(false), initialized(false)
{
}

CharacterRenderer::~CharacterRenderer()
{
    shutdown();
}

void CharacterRenderer::initialize(RenderContext* ctx)
{
    this->renderContext = ctx;
    initialized = true;
}

void CharacterRenderer::shutdown()
{
    renderContext = nullptr;
    initialized = false;
}

CharacterMesh CharacterRenderer::generateMesh(const VoxelCharacter& character)
{
    CharacterMesh mesh;

    // Generate vertex data from character voxels
    generateVertices(character, mesh.vertices, mesh.indices);

    mesh.vertexCount =
        static_cast<uint32_t>(mesh.vertices.size() / 6); // 6 floats per vertex (pos + color)
    mesh.indexCount = static_cast<uint32_t>(mesh.indices.size());

    // Create GPU buffers if render context is available
    if (initialized && renderContext) {
        createBuffers(mesh);
    }

    return mesh;
}

void CharacterRenderer::updateMesh(const VoxelCharacter& character, CharacterMesh& mesh)
{
    // Release old buffers
    releaseMesh(mesh);

    // Generate new mesh
    CharacterMesh newMesh = generateMesh(character);
    mesh = newMesh;
}

void CharacterRenderer::render(const CharacterMesh& mesh, const glm::mat4& worldTransform)
{
    if (!initialized || !renderContext) {
        return;
    }

    // Render the mesh using the render context
    // This would call platform-specific rendering code
    // For now, this is a stub
    (void)mesh;
    (void)worldTransform;
}

void CharacterRenderer::releaseMesh(CharacterMesh& mesh)
{
    // Release GPU buffers
    // Platform-specific cleanup would go here
    mesh.vertexBuffer = nullptr;
    mesh.indexBuffer = nullptr;
    mesh.vertices.clear();
    mesh.indices.clear();
    mesh.vertexCount = 0;
    mesh.indexCount = 0;
}

void CharacterRenderer::generateVertices(const VoxelCharacter& character,
                                         std::vector<float>& outVertices,
                                         std::vector<uint32_t>& outIndices)
{
    outVertices.clear();
    outIndices.clear();

    // Get transformed voxels with bone animations applied
    std::vector<CharacterVoxel> voxels = character.getTransformedVoxels();

    // Generate a cube for each voxel
    for (const auto& voxel : voxels) {
        addVoxelCube(voxel.position, voxel.color, outVertices, outIndices);
    }
}

void CharacterRenderer::addVoxelCube(const glm::ivec3& position, const glm::vec3& color,
                                     std::vector<float>& vertices, std::vector<uint32_t>& indices)
{
    const float x = static_cast<float>(position.x);
    const float y = static_cast<float>(position.y);
    const float z = static_cast<float>(position.z);
    const float size = 1.0f;

    uint32_t baseIndex = static_cast<uint32_t>(vertices.size() / 6);

    // Define 8 cube vertices with position (3 floats) and color (3 floats)
    float cubeVertices[] = {
        // Front face
        x,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,
        x,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,

        // Back face
        x,
        y,
        z,
        color.r,
        color.g,
        color.b,
        x + size,
        y,
        z,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z,
        color.r,
        color.g,
        color.b,
        x,
        y + size,
        z,
        color.r,
        color.g,
        color.b,

        // Left face
        x,
        y,
        z,
        color.r,
        color.g,
        color.b,
        x,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,
        x,
        y + size,
        z,
        color.r,
        color.g,
        color.b,

        // Right face
        x + size,
        y,
        z,
        color.r,
        color.g,
        color.b,
        x + size,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z,
        color.r,
        color.g,
        color.b,

        // Top face
        x,
        y + size,
        z,
        color.r,
        color.g,
        color.b,
        x,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y + size,
        z,
        color.r,
        color.g,
        color.b,

        // Bottom face
        x,
        y,
        z,
        color.r,
        color.g,
        color.b,
        x,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y,
        z + size,
        color.r,
        color.g,
        color.b,
        x + size,
        y,
        z,
        color.r,
        color.g,
        color.b,
    };

    // Add vertices
    for (size_t i = 0; i < sizeof(cubeVertices) / sizeof(float); ++i) {
        vertices.push_back(cubeVertices[i]);
    }

    // Define cube indices (2 triangles per face, 6 faces)
    uint32_t cubeIndices[] = {
        // Front
        baseIndex + 0,
        baseIndex + 1,
        baseIndex + 2,
        baseIndex + 2,
        baseIndex + 3,
        baseIndex + 0,
        // Back
        baseIndex + 5,
        baseIndex + 4,
        baseIndex + 7,
        baseIndex + 7,
        baseIndex + 6,
        baseIndex + 5,
        // Left
        baseIndex + 8,
        baseIndex + 9,
        baseIndex + 10,
        baseIndex + 10,
        baseIndex + 11,
        baseIndex + 8,
        // Right
        baseIndex + 12,
        baseIndex + 13,
        baseIndex + 14,
        baseIndex + 14,
        baseIndex + 15,
        baseIndex + 12,
        // Top
        baseIndex + 16,
        baseIndex + 17,
        baseIndex + 18,
        baseIndex + 18,
        baseIndex + 19,
        baseIndex + 16,
        // Bottom
        baseIndex + 21,
        baseIndex + 20,
        baseIndex + 23,
        baseIndex + 23,
        baseIndex + 22,
        baseIndex + 21,
    };

    // Add indices
    for (size_t i = 0; i < sizeof(cubeIndices) / sizeof(uint32_t); ++i) {
        indices.push_back(cubeIndices[i]);
    }
}

void CharacterRenderer::createBuffers(CharacterMesh& mesh)
{
    // Platform-specific buffer creation would go here
    // For DirectX, would create ID3D11Buffer or ID3D12Resource
    // For now, this is a stub
    (void)mesh;
}

// CharacterRenderManager implementation

CharacterRenderManager::CharacterRenderManager() : renderer(nullptr), nextInstanceId(0) {}

CharacterRenderManager::~CharacterRenderManager()
{
    clear();
}

void CharacterRenderManager::initialize(CharacterRenderer* charRenderer)
{
    this->renderer = charRenderer;
}

int CharacterRenderManager::registerCharacter(VoxelCharacter* character)
{
    CharacterInstance instance;
    instance.character = character;
    instance.needsUpdate = true;

    int instanceId = nextInstanceId++;
    instances.push_back(instance);

    return instanceId;
}

void CharacterRenderManager::unregisterCharacter(int instanceId)
{
    if (instanceId >= 0 && instanceId < static_cast<int>(instances.size())) {
        // Release mesh resources
        if (renderer) {
            renderer->releaseMesh(instances[instanceId].mesh);
        }

        // Remove from instances (simple removal, could be optimized)
        instances.erase(instances.begin() + instanceId);
    }
}

void CharacterRenderManager::updateCharacter(int instanceId)
{
    if (instanceId >= 0 && instanceId < static_cast<int>(instances.size())) {
        instances[instanceId].needsUpdate = true;
    }
}

void CharacterRenderManager::renderAll(const glm::mat4& viewMatrix,
                                       const glm::mat4& projectionMatrix)
{
    if (!renderer) {
        return;
    }

    for (auto& instance : instances) {
        // Update mesh if needed
        if (instance.needsUpdate && instance.character) {
            renderer->updateMesh(*instance.character, instance.mesh);
            instance.needsUpdate = false;
        }

        // Render character
        if (instance.character) {
            glm::mat4 worldTransform(1.0f);
            worldTransform = glm::translate(worldTransform, instance.character->getPosition());

            renderer->render(instance.mesh, worldTransform);
        }
    }

    // Suppress unused parameter warnings
    (void)viewMatrix;
    (void)projectionMatrix;
}

void CharacterRenderManager::clear()
{
    if (renderer) {
        for (auto& instance : instances) {
            renderer->releaseMesh(instance.mesh);
        }
    }
    instances.clear();
    nextInstanceId = 0;
}

} // namespace fresh
