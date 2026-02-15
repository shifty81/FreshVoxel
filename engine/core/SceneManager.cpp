#include "core/SceneManager.h"

#include <iostream>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#ifdef FRESH_JSON_AVAILABLE
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

namespace fresh
{

// SceneNode implementation
SceneNode::SceneNode(const std::string& name) : name(name) {}

SceneNode::~SceneNode() {}

void SceneNode::addChild(std::shared_ptr<SceneNode> child)
{
    if (child) {
        child->parent = this;
        children.push_back(child);
    }
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->parent = nullptr;
        children.erase(it);
    }
}

glm::mat4 SceneNode::getLocalTransform() const
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform *= glm::mat4_cast(rotation);
    transform = glm::scale(transform, scale);
    return transform;
}

glm::mat4 SceneNode::getWorldTransform() const
{
    if (parent) {
        return parent->getWorldTransform() * getLocalTransform();
    }
    return getLocalTransform();
}

void SceneNode::update(float deltaTime)
{
    if (!active)
        return;

    for (auto& child : children) {
        if (child) {
            child->update(deltaTime);
        }
    }
}

void SceneNode::render()
{
    if (!active)
        return;

    for (auto& child : children) {
        if (child) {
            child->render();
        }
    }
}

// Scene implementation
Scene::Scene(const std::string& name) : name(name)
{
    root = std::make_shared<SceneNode>("Root");
}

Scene::~Scene()
{
    shutdown();
}

void Scene::initialize()
{
    std::cout << "Initializing scene: " << name << std::endl;
    loaded = true;
}

void Scene::shutdown()
{
    std::cout << "Shutting down scene: " << name << std::endl;
    root->clearChildren();
    loaded = false;
}

void Scene::update(float deltaTime)
{
    if (loaded && root) {
        root->update(deltaTime);
    }
}

void Scene::render()
{
    if (loaded && root) {
        root->render();
    }
}

void Scene::addNode(std::shared_ptr<SceneNode> node)
{
    if (root && node) {
        root->addChild(node);
    }
}

SceneNode* Scene::findNode(const std::string& nodeName)
{
    // Simple recursive search (could be optimized with a map)
    std::function<SceneNode*(SceneNode*, const std::string&)> search;
    search = [&](SceneNode* node, const std::string& targetName) -> SceneNode* {
        if (!node)
            return nullptr;
        if (node->getName() == targetName)
            return node;

        for (auto& child : node->getChildren()) {
            SceneNode* result = search(child.get(), targetName);
            if (result)
                return result;
        }
        return nullptr;
    };

    return search(root.get(), nodeName);
}

// SceneManager implementation
SceneManager& SceneManager::getInstance()
{
    static SceneManager instance;
    return instance;
}

Scene* SceneManager::createScene(const std::string& name)
{
    auto scene = std::make_unique<Scene>(name);
    Scene* ptr = scene.get();
    scenes[name] = std::move(scene);

    std::cout << "Created scene: " << name << std::endl;
    return ptr;
}

Scene* SceneManager::loadScene(const std::string& path)
{
#ifdef FRESH_JSON_AVAILABLE
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open scene file: " << path << std::endl;
            return nullptr;
        }

        json sceneData;
        file >> sceneData;
        file.close();

        // Get scene name
        std::string sceneName = sceneData.value("name", "Unnamed Scene");
        
        // Create new scene
        Scene* scene = createScene(sceneName);
        if (!scene) {
            std::cerr << "Failed to create scene: " << sceneName << std::endl;
            return nullptr;
        }

        // Load metadata
        if (sceneData.contains("metadata")) {
            [[maybe_unused]] auto& metadata = sceneData["metadata"];
            // Future: load scene-specific metadata
        }

        // Load scene nodes recursively
        if (sceneData.contains("root")) {
            loadSceneNode(scene->getRoot().get(), sceneData["root"]);
        }

        std::cout << "Successfully loaded scene from: " << path << std::endl;
        return scene;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return nullptr;
    }
#else
    std::cout << "Scene loading requires nlohmann-json library. Path: " << path << std::endl;
    return nullptr;
#endif
}

bool SceneManager::saveScene(const std::string& path, Scene* scene)
{
    if (!scene)
        return false;

#ifdef FRESH_JSON_AVAILABLE
    try {
        json sceneData;
        
        // Save scene metadata
        sceneData["name"] = scene->getName();
        sceneData["version"] = "1.0";
        sceneData["metadata"] = json::object();

        // Save scene graph starting from root
        if (scene->getRoot()) {
            sceneData["root"] = saveSceneNode(scene->getRoot().get());
        }

        // Write to file with pretty printing
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }

        file << sceneData.dump(2); // 2-space indentation
        file.close();

        std::cout << "Successfully saved scene to: " << path << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving scene: " << e.what() << std::endl;
        return false;
    }
#else
    std::cout << "Scene saving requires nlohmann-json library. Path: " << path << std::endl;
    return false;
#endif
}

void SceneManager::setActiveScene(const std::string& name)
{
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        activeScene = it->second.get();
        if (!activeScene->isLoaded()) {
            activeScene->initialize();
        }
        std::cout << "Set active scene: " << name << std::endl;
    }
}

void SceneManager::unloadScene(const std::string& name)
{
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        if (it->second.get() == activeScene) {
            activeScene = nullptr;
        }
        it->second->shutdown();
        scenes.erase(it);
        std::cout << "Unloaded scene: " << name << std::endl;
    }
}

Scene* SceneManager::getScene(const std::string& name)
{
    auto it = scenes.find(name);
    return (it != scenes.end()) ? it->second.get() : nullptr;
}

void SceneManager::update(float deltaTime)
{
    if (activeScene) {
        activeScene->update(deltaTime);
    }

    // Handle transitions
    if (transitioning) {
        transitionTime += deltaTime;
        if (transitionTime >= transitionDuration) {
            setActiveScene(targetScene);
            transitioning = false;
            transitionTime = 0.0f;
        }
    }
}

void SceneManager::render()
{
    if (activeScene) {
        activeScene->render();
    }
}

void SceneManager::transitionTo(const std::string& sceneName, float fadeTime)
{
    transitioning = true;
    targetScene = sceneName;
    transitionDuration = fadeTime;
    transitionTime = 0.0f;

    std::cout << "Transitioning to scene: " << sceneName << std::endl;
}

#ifdef FRESH_JSON_AVAILABLE
json SceneManager::saveSceneNode(SceneNode* node)
{
    if (!node) {
        return json::object();
    }

    json nodeData;
    
    // Basic properties
    nodeData["name"] = node->getName();
    nodeData["active"] = node->isActive();

    // Transform
    glm::vec3 pos = node->getPosition();
    glm::quat rot = node->getRotation();
    glm::vec3 scl = node->getScale();

    nodeData["position"] = {pos.x, pos.y, pos.z};
    nodeData["rotation"] = {rot.x, rot.y, rot.z, rot.w};
    nodeData["scale"] = {scl.x, scl.y, scl.z};

    // Children
    auto& children = node->getChildren();
    if (!children.empty()) {
        json childrenArray = json::array();
        for (const auto& child : children) {
            if (child) {
                childrenArray.push_back(saveSceneNode(child.get()));
            }
        }
        nodeData["children"] = childrenArray;
    }

    // Component data (if any)
    // Future enhancement: serialize attached components

    return nodeData;
}

void SceneManager::loadSceneNode(SceneNode* parent, const json& nodeData)
{
    if (!parent || nodeData.is_null()) {
        return;
    }

    // Load node properties
    if (nodeData.contains("active")) {
        parent->setActive(nodeData["active"].get<bool>());
    }

    // Load transform
    if (nodeData.contains("position")) {
        auto pos = nodeData["position"];
        if (pos.is_array() && pos.size() == 3) {
            parent->setPosition(glm::vec3(
                pos[0].get<float>(),
                pos[1].get<float>(),
                pos[2].get<float>()
            ));
        }
    }

    if (nodeData.contains("rotation")) {
        auto rot = nodeData["rotation"];
        if (rot.is_array() && rot.size() == 4) {
            parent->setRotation(glm::quat(
                rot[3].get<float>(),  // w
                rot[0].get<float>(),  // x
                rot[1].get<float>(),  // y
                rot[2].get<float>()   // z
            ));
        }
    }

    if (nodeData.contains("scale")) {
        auto scl = nodeData["scale"];
        if (scl.is_array() && scl.size() == 3) {
            parent->setScale(glm::vec3(
                scl[0].get<float>(),
                scl[1].get<float>(),
                scl[2].get<float>()
            ));
        }
    }

    // Load children recursively
    if (nodeData.contains("children")) {
        const auto& childrenArray = nodeData["children"];
        if (childrenArray.is_array()) {
            for (const auto& childData : childrenArray) {
                if (childData.contains("name")) {
                    std::string childName = childData["name"].get<std::string>();
                    auto child = std::make_shared<SceneNode>(childName);
                    parent->addChild(child);
                    loadSceneNode(child.get(), childData);
                }
            }
        }
    }

    // Component loading (future enhancement)
}
#endif

} // namespace fresh
