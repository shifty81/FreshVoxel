#include "editor/PrefabSystem.h"

#include "ecs/EntityManager.h"
#include "ecs/TransformComponent.h"
#include "ecs/HealthComponent.h"
#include "ecs/MovementComponent.h"
#include "core/Logger.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace fresh
{

// ==========================================================================
// Internal JSON helpers
// ==========================================================================

std::string PrefabSystem::escapeString(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '"':  out += "\\\""; break;
        case '\n': out += "\\n";  break;
        case '\r': out += "\\r";  break;
        case '\t': out += "\\t";  break;
        default:   out += c;      break;
        }
    }
    return out;
}

std::string PrefabSystem::vec3ToJSON(float x, float y, float z)
{
    std::ostringstream ss;
    ss << "[" << x << ", " << y << ", " << z << "]";
    return ss.str();
}

std::string PrefabSystem::vec4ToJSON(float x, float y, float z, float w)
{
    std::ostringstream ss;
    ss << "[" << x << ", " << y << ", " << z << ", " << w << "]";
    return ss.str();
}

// Find the first occurrence of "key" followed by a colon and a numeric array
// of `count` floats, e.g. "position": [1.0, 2.0, 3.0]
bool PrefabSystem::parseFloatArray(const std::string& json,
                                   const std::string& key,
                                   float* out, int count)
{
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;
    pos += needle.size();

    // Skip to '['
    pos = json.find('[', pos);
    if (pos == std::string::npos) return false;
    ++pos;

    for (int i = 0; i < count; ++i) {
        // Skip whitespace and commas
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == ',' || json[pos] == '\t'))
            ++pos;
        if (pos >= json.size() || json[pos] == ']') return false;

        char* end = nullptr;
        out[i] = std::strtof(json.c_str() + pos, &end);
        if (end == json.c_str() + pos) return false;
        pos = static_cast<size_t>(end - json.c_str());
    }
    return true;
}

bool PrefabSystem::parseFloat(const std::string& json, const std::string& key, float& out)
{
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return false;
    pos += needle.size();

    // Skip to ':'
    pos = json.find(':', pos);
    if (pos == std::string::npos) return false;
    ++pos;

    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    char* end = nullptr;
    out = std::strtof(json.c_str() + pos, &end);
    return end != json.c_str() + pos;
}

std::string PrefabSystem::parseString(const std::string& json, const std::string& key)
{
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();

    pos = json.find('"', pos); // opening quote of value
    if (pos == std::string::npos) return {};
    ++pos;

    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
            switch (json[pos]) {
            case '\\': result += '\\'; break;
            case '"':  result += '"';  break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            default:   result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        ++pos;
    }
    return result;
}

// Split the "components" array into per-component JSON blocks
std::vector<std::string> PrefabSystem::splitComponentBlocks(const std::string& json)
{
    std::vector<std::string> blocks;

    size_t arrStart = json.find("\"components\"");
    if (arrStart == std::string::npos) return blocks;
    arrStart = json.find('[', arrStart);
    if (arrStart == std::string::npos) return blocks;
    ++arrStart;

    int depth = 0;
    size_t blockStart = std::string::npos;

    for (size_t i = arrStart; i < json.size(); ++i) {
        char c = json[i];
        if (c == '{') {
            if (depth == 0) blockStart = i;
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && blockStart != std::string::npos) {
                blocks.push_back(json.substr(blockStart, i - blockStart + 1));
                blockStart = std::string::npos;
            }
        } else if (c == ']' && depth == 0) {
            break; // end of components array
        }
    }
    return blocks;
}

std::string PrefabSystem::extractComponentType(const std::string& block)
{
    return parseString(block, "type");
}

// ==========================================================================
// Serialization
// ==========================================================================

std::string PrefabSystem::serializeEntity(const ecs::EntityManager& em,
                                           ecs::Entity entity,
                                           const std::string& name) const
{
    if (!em.isEntityValid(entity)) {
        Logger::getInstance().error("PrefabSystem::serializeEntity — invalid entity", "PrefabSystem");
        return {};
    }

    std::ostringstream ss;
    ss << "{\n"
       << "  \"prefab_version\": 1,\n"
       << "  \"name\": \"" << escapeString(name) << "\",\n"
       << "  \"entity_id\": " << entity.getId() << ",\n"
       << "  \"components\": [\n";

    bool firstComp = true;
    auto comma = [&]() -> const char* { if (!firstComp) return ",\n"; firstComp = false; return ""; };

    // TransformComponent
    const auto* tc = em.getComponent<ecs::TransformComponent>(entity);
    if (tc) {
        const glm::quat& q = tc->rotation;
        ss << comma()
           << "    {\n"
           << "      \"type\": \"TransformComponent\",\n"
           << "      \"position\": " << vec3ToJSON(tc->position.x, tc->position.y, tc->position.z) << ",\n"
           << "      \"rotation\": " << vec4ToJSON(q.x, q.y, q.z, q.w) << ",\n"
           << "      \"scale\":    " << vec3ToJSON(tc->scale.x, tc->scale.y, tc->scale.z) << "\n"
           << "    }";
    }

    // HealthComponent
    const auto* hc = em.getComponent<ecs::HealthComponent>(entity);
    if (hc) {
        ss << comma()
           << "    {\n"
           << "      \"type\": \"HealthComponent\",\n"
           << "      \"maxHealth\": "     << hc->getMaxHealth()     << ",\n"
           << "      \"currentHealth\": " << hc->getCurrentHealth() << "\n"
           << "    }";
    }

    // MovementComponent
    const auto* mc = em.getComponent<ecs::MovementComponent>(entity);
    if (mc) {
        ss << comma()
           << "    {\n"
           << "      \"type\": \"MovementComponent\",\n"
           << "      \"maxSpeed\":     " << mc->getMaxSpeed()     << ",\n"
           << "      \"acceleration\": " << mc->getAcceleration() << "\n"
           << "    }";
    }

    ss << "\n  ]\n}\n";
    return ss.str();
}

bool PrefabSystem::savePrefab(const std::string& filePath, const std::string& json) const
{
    if (json.empty()) return false;

    std::ofstream out(filePath, std::ios::out | std::ios::trunc);
    if (!out.is_open()) {
        Logger::getInstance().error("PrefabSystem: cannot open '" + filePath + "' for writing",
                                    "PrefabSystem");
        return false;
    }
    out << json;
    Logger::getInstance().info("Prefab saved: " + filePath, "PrefabSystem");
    return true;
}

bool PrefabSystem::saveEntityAsPrefab(const ecs::EntityManager& em,
                                       ecs::Entity entity,
                                       const std::string& filePath) const
{
    // Derive prefab name from file stem
    std::string name = filePath;
    size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    size_t dot = name.rfind('.');
    if (dot != std::string::npos) name = name.substr(0, dot);

    const std::string json = serializeEntity(em, entity, name);
    return savePrefab(filePath, json);
}

// ==========================================================================
// Deserialization
// ==========================================================================

std::string PrefabSystem::loadPrefab(const std::string& filePath) const
{
    std::ifstream in(filePath);
    if (!in.is_open()) {
        Logger::getInstance().error("PrefabSystem: cannot open '" + filePath + "' for reading",
                                    "PrefabSystem");
        return {};
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

ecs::Entity PrefabSystem::spawnFromJSON(ecs::EntityManager& em,
                                         const std::string& json) const
{
    if (json.empty()) {
        Logger::getInstance().error("PrefabSystem::spawnFromJSON — empty JSON", "PrefabSystem");
        return ecs::Entity{}; // invalid
    }

    const ecs::Entity entity = em.createEntity();

    const auto componentBlocks = splitComponentBlocks(json);
    for (const auto& block : componentBlocks) {
        const std::string type = extractComponentType(block);

        if (type == "TransformComponent") {
            float pos[3] = {0, 0, 0};
            float rot[4] = {0, 0, 0, 1};
            float scl[3] = {1, 1, 1};
            parseFloatArray(block, "position", pos, 3);
            parseFloatArray(block, "rotation", rot, 4);
            parseFloatArray(block, "scale",    scl, 3);

            auto* tc = new ecs::TransformComponent();
            tc->position = glm::vec3(pos[0], pos[1], pos[2]);
            tc->rotation = glm::quat(rot[3], rot[0], rot[1], rot[2]); // glm quat: w,x,y,z
            tc->scale    = glm::vec3(scl[0], scl[1], scl[2]);
            em.addComponent(entity, tc);

        } else if (type == "HealthComponent") {
            float maxH = 100.0f;
            float curH = 100.0f;
            parseFloat(block, "maxHealth",     maxH);
            parseFloat(block, "currentHealth", curH);

            auto* hc = new ecs::HealthComponent(maxH);
            // Restore current health by dealing the difference as "damage"
            if (curH < maxH)
                hc->takeDamage(maxH - curH);
            em.addComponent(entity, hc);

        } else if (type == "MovementComponent") {
            float spd = 5.0f;
            float acc = 10.0f;
            parseFloat(block, "maxSpeed",     spd);
            parseFloat(block, "acceleration", acc);

            em.addComponent(entity, new ecs::MovementComponent(spd, acc));

        } else if (!type.empty()) {
            Logger::getInstance().warning(
                "PrefabSystem: unknown component type '" + type + "' — skipped",
                "PrefabSystem");
        }
    }

    const std::string name = parseString(json, "name");
    Logger::getInstance().info(
        "Prefab '" + name + "' spawned as entity " + std::to_string(entity.getId()),
        "PrefabSystem");
    return entity;
}

ecs::Entity PrefabSystem::spawnPrefab(ecs::EntityManager& em,
                                       const std::string& filePath) const
{
    const std::string json = loadPrefab(filePath);
    if (json.empty()) return ecs::Entity{};
    return spawnFromJSON(em, json);
}

// ==========================================================================
// Listing
// ==========================================================================

std::vector<std::string> PrefabSystem::listPrefabs(const std::string& directory) const
{
    std::vector<std::string> result;

#ifdef _WIN32
    // Windows directory enumeration
    std::string pattern = directory + "\\*.prefab";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            result.push_back(directory + "\\" + findData.cFileName);
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(directory.c_str());
    if (dir) {
        struct dirent* ent = nullptr;
        while ((ent = readdir(dir)) != nullptr) {
            std::string fname = ent->d_name;
            if (fname.size() > 7 && fname.substr(fname.size() - 7) == ".prefab") {
                result.push_back(directory + "/" + fname);
            }
        }
        closedir(dir);
    }
#endif

    std::sort(result.begin(), result.end());
    return result;
}

} // namespace fresh
