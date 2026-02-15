#pragma once
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#ifdef FRESH_JSON_AVAILABLE
#include <nlohmann/json_fwd.hpp>
#endif

namespace fresh
{

/**
 * @brief Scene node in the scene graph
 */
class SceneNode
{
public:
    SceneNode(const std::string& name = "Node");
    virtual ~SceneNode();

    // Hierarchy management
    void addChild(std::shared_ptr<SceneNode> child);
    void removeChild(std::shared_ptr<SceneNode> child);
    void clearChildren()
    {
        children.clear();
    }
    SceneNode* getParent() const
    {
        return parent;
    }
    const std::vector<std::shared_ptr<SceneNode>>& getChildren() const
    {
        return children;
    }

    // Transform
    void setPosition(const glm::vec3& pos)
    {
        position = pos;
    }
    void setRotation(const glm::quat& rot)
    {
        rotation = rot;
    }
    void setScale(const glm::vec3& scl)
    {
        scale = scl;
    }

    glm::vec3 getPosition() const
    {
        return position;
    }
    glm::quat getRotation() const
    {
        return rotation;
    }
    glm::vec3 getScale() const
    {
        return scale;
    }

    glm::mat4 getLocalTransform() const;
    glm::mat4 getWorldTransform() const;

    // Properties
    const std::string& getName() const
    {
        return name;
    }
    void setName(const std::string& n)
    {
        name = n;
    }

    bool isActive() const
    {
        return active;
    }
    void setActive(bool a)
    {
        active = a;
    }

    // Update
    virtual void update(float deltaTime);
    virtual void render();

protected:
    std::string name;
    SceneNode* parent = nullptr;
    std::vector<std::shared_ptr<SceneNode>> children;

    glm::vec3 position{0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    bool active = true;
};

/**
 * @brief Scene containing a hierarchy of game objects
 */
class Scene
{
public:
    Scene(const std::string& name);
    ~Scene();

    // Lifecycle
    void initialize();
    void shutdown();
    void update(float deltaTime);
    void render();

    // Scene graph
    std::shared_ptr<SceneNode> getRoot()
    {
        return root;
    }
    void addNode(std::shared_ptr<SceneNode> node);
    SceneNode* findNode(const std::string& name);

    // Properties
    const std::string& getName() const
    {
        return name;
    }
    bool isLoaded() const
    {
        return loaded;
    }

private:
    std::string name;
    std::shared_ptr<SceneNode> root;
    bool loaded = false;
};

/**
 * @brief Manages scenes and transitions between them
 *
 * Handles loading/unloading scenes, scene transitions,
 * and maintains the active scene.
 */
class SceneManager
{
public:
    static SceneManager& getInstance();

    /**
     * @brief Create and register a new scene
     */
    Scene* createScene(const std::string& name);

    /**
     * @brief Load a scene from file
     */
    Scene* loadScene(const std::string& path);

    /**
     * @brief Save scene to file
     */
    bool saveScene(const std::string& path, Scene* scene);

    /**
     * @brief Set active scene
     */
    void setActiveScene(const std::string& name);
    Scene* getActiveScene()
    {
        return activeScene;
    }

    /**
     * @brief Unload a scene
     */
    void unloadScene(const std::string& name);

    /**
     * @brief Get scene by name
     */
    Scene* getScene(const std::string& name);

    /**
     * @brief Update active scene
     */
    void update(float deltaTime);

    /**
     * @brief Render active scene
     */
    void render();

    /**
     * @brief Transition to new scene with fade
     */
    void transitionTo(const std::string& sceneName, float fadeTime = 0.5f);

private:
    SceneManager() = default;
    ~SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::map<std::string, std::unique_ptr<Scene>> scenes;
    Scene* activeScene = nullptr;

    // Transition state
    bool transitioning = false;
    std::string targetScene;
    float transitionTime = 0.0f;
    float transitionDuration = 0.5f;

#ifdef FRESH_JSON_AVAILABLE
    // Serialization helpers (implementation in .cpp file)
    nlohmann::json saveSceneNode(SceneNode* node);
    void loadSceneNode(SceneNode* parent, const nlohmann::json& nodeData);
#endif
};

} // namespace fresh
