#pragma once

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

class Material;
class RenderBuffer;

/**
 * @brief Vertex structure for 3D models
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};
};

/**
 * @brief Mesh data structure
 */
class Mesh
{
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
    ~Mesh() = default;

    const std::vector<Vertex>& getVertices() const
    {
        return vertices;
    }
    const std::vector<uint32_t>& getIndices() const
    {
        return indices;
    }

    void setMaterial(std::shared_ptr<Material> mat)
    {
        material = mat;
    }
    std::shared_ptr<Material> getMaterial() const
    {
        return material;
    }

    // GPU buffers (created by renderer)
    void setVertexBuffer(std::shared_ptr<RenderBuffer> vb)
    {
        vertexBuffer = vb;
    }
    void setIndexBuffer(std::shared_ptr<RenderBuffer> ib)
    {
        indexBuffer = ib;
    }
    std::shared_ptr<RenderBuffer> getVertexBuffer() const
    {
        return vertexBuffer;
    }
    std::shared_ptr<RenderBuffer> getIndexBuffer() const
    {
        return indexBuffer;
    }

    // Bounding box
    glm::vec3 getMin() const
    {
        return boundsMin;
    }
    glm::vec3 getMax() const
    {
        return boundsMax;
    }
    glm::vec3 getCenter() const
    {
        return (boundsMin + boundsMax) * 0.5f;
    }

private:
    void calculateBounds();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::shared_ptr<Material> material;

    std::shared_ptr<RenderBuffer> vertexBuffer;
    std::shared_ptr<RenderBuffer> indexBuffer;

    glm::vec3 boundsMin{0.0f};
    glm::vec3 boundsMax{0.0f};
};

/**
 * @brief 3D Model containing multiple meshes
 */
class Model
{
public:
    Model() = default;
    ~Model() = default;

    /**
     * @brief Add mesh to model
     */
    void addMesh(std::shared_ptr<Mesh> mesh)
    {
        meshes.push_back(mesh);
    }

    /**
     * @brief Get all meshes
     */
    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const
    {
        return meshes;
    }

    /**
     * @brief Get bounding box of entire model
     */
    glm::vec3 getMin() const;
    glm::vec3 getMax() const;
    glm::vec3 getCenter() const;

    /**
     * @brief Get model path (if loaded from file)
     */
    const std::string& getPath() const
    {
        return path;
    }
    void setPath(const std::string& p)
    {
        path = p;
    }

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::string path;
};

/**
 * @brief Model loader with Assimp support
 *
 * Supports loading .obj, .fbx, .gltf, .dae, and many other formats.
 */
class ModelLoader
{
public:
    /**
     * @brief Load model from file
     * @param path Path to model file
     * @param generateTangents Generate tangent space for normal mapping
     * @return Loaded model or nullptr on failure
     */
    static std::shared_ptr<Model> loadModel(const std::string& path, bool generateTangents = true);

    /**
     * @brief Load model from file with options
     * @param path Path to model file
     * @param triangulate Convert all polygons to triangles
     * @param generateNormals Generate normals if missing
     * @param generateTangents Generate tangent space
     * @param flipUVs Flip UV coordinates vertically
     * @param optimizeMeshes Optimize mesh structure
     * @return Loaded model or nullptr on failure
     */
    static std::shared_ptr<Model>
    loadModelWithOptions(const std::string& path, bool triangulate = true,
                         bool generateNormals = true, bool generateTangents = true,
                         bool flipUVs = false, bool optimizeMeshes = true);

    /**
     * @brief Create primitive shapes
     */
    static std::shared_ptr<Model> createCube(float size = 1.0f);
    static std::shared_ptr<Model> createSphere(float radius = 1.0f, int segments = 32);
    static std::shared_ptr<Model> createPlane(float width = 1.0f, float height = 1.0f);
    static std::shared_ptr<Model> createCylinder(float radius = 1.0f, float height = 2.0f,
                                                 int segments = 32);
    static std::shared_ptr<Model> createCone(float radius = 1.0f, float height = 2.0f,
                                             int segments = 32);

private:
    // Internal helper functions
    static std::shared_ptr<Mesh> processMesh(const void* aiMesh, const void* aiScene);
    static std::shared_ptr<Material> processMaterial(const void* aiMaterial,
                                                     const std::string& directory);
    static void calculateTangents(std::vector<Vertex>& vertices,
                                  const std::vector<uint32_t>& indices);
};

} // namespace fresh
