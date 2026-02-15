#include "assets/VoxelMaterialPack.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#ifdef FRESH_JSON_AVAILABLE
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

#include "core/Logger.h"
#include "renderer/TextureManager.h"

namespace fresh
{

// VoxelMaterialPack implementation

VoxelMaterialPack::VoxelMaterialPack(const std::string& packPath) : packPath(packPath) {}

VoxelMaterialPack::~VoxelMaterialPack()
{
    unload();
}

bool VoxelMaterialPack::load()
{
    if (loaded)
        return true;

    LOG_INFO("Loading voxel material pack from: " + packPath);

    // Look for manifest file
    std::string manifestPath = packPath + "/voxel_materials.json";
    if (!std::filesystem::exists(manifestPath)) {
        // Try alternate name
        manifestPath = packPath + "/materials.json";
        if (!std::filesystem::exists(manifestPath)) {
            LOG_ERROR("Manifest not found in: " + packPath);
            return false;
        }
    }

    if (!parseManifest(manifestPath)) {
        LOG_ERROR("Failed to parse manifest: " + manifestPath);
        return false;
    }

    loaded = true;
    LOG_INFO("Loaded voxel material pack: " + name + " v" + version);
    LOG_INFO("  Material definitions: " + std::to_string(materialDefinitions.size()));
    return true;
}

void VoxelMaterialPack::unload()
{
    if (!loaded)
        return;

    LOG_INFO("Unloading voxel material pack: " + name);
    materialDefinitions.clear();
    voxelTypeToDefIndex.clear();
    textureCache.clear();
    textureAtlas.reset();
    atlasMapping.clear();
    loaded = false;
}

VoxelType VoxelMaterialPack::parseVoxelType(const std::string& typeStr) const
{
    // Map string to VoxelType enum
    static const std::map<std::string, VoxelType> typeMap = {
        {"Air", VoxelType::Air},
        {"Stone", VoxelType::Stone},
        {"Dirt", VoxelType::Dirt},
        {"Grass", VoxelType::Grass},
        {"Sand", VoxelType::Sand},
        {"Water", VoxelType::Water},
        {"Wood", VoxelType::Wood},
        {"Leaves", VoxelType::Leaves},
        {"Bedrock", VoxelType::Bedrock},
        {"Snow", VoxelType::Snow},
        {"Ice", VoxelType::Ice},
        {"Cobblestone", VoxelType::Cobblestone},
        {"Planks", VoxelType::Planks},
        {"Glass", VoxelType::Glass}
    };

    auto it = typeMap.find(typeStr);
    if (it != typeMap.end()) {
        return it->second;
    }

    LOG_WARNING("Unknown voxel type: " + typeStr + ", defaulting to Stone");
    return VoxelType::Stone;
}

std::string VoxelMaterialPack::voxelTypeToString(VoxelType type) const
{
    switch (type) {
        case VoxelType::Air: return "Air";
        case VoxelType::Stone: return "Stone";
        case VoxelType::Dirt: return "Dirt";
        case VoxelType::Grass: return "Grass";
        case VoxelType::Sand: return "Sand";
        case VoxelType::Water: return "Water";
        case VoxelType::Wood: return "Wood";
        case VoxelType::Leaves: return "Leaves";
        case VoxelType::Bedrock: return "Bedrock";
        case VoxelType::Snow: return "Snow";
        case VoxelType::Ice: return "Ice";
        case VoxelType::Cobblestone: return "Cobblestone";
        case VoxelType::Planks: return "Planks";
        case VoxelType::Glass: return "Glass";
        default: return "Unknown";
    }
}

bool VoxelMaterialPack::parseManifest(const std::string& manifestPath)
{
#ifdef FRESH_JSON_AVAILABLE
    try {
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open manifest file: " + manifestPath);
            return false;
        }

        json manifestJson;
        file >> manifestJson;
        file.close();

        // Parse pack metadata
        name = manifestJson.value("name", std::filesystem::path(packPath).filename().string());
        version = manifestJson.value("version", "1.0.0");
        author = manifestJson.value("author", "Unknown");
        description = manifestJson.value("description", "");
        priority = manifestJson.value("priority", 0);

        // Parse material definitions
        if (manifestJson.contains("materials") && manifestJson["materials"].is_array()) {
            for (const auto& matJson : manifestJson["materials"]) {
                VoxelMaterialDefinition matDef;

                // Parse voxel type
                std::string typeStr = matJson.value("voxelType", "Stone");
                matDef.voxelType = parseVoxelType(typeStr);
                matDef.name = matJson.value("name", voxelTypeToString(matDef.voxelType));

                // Parse textures
                if (matJson.contains("textures")) {
                    const auto& texJson = matJson["textures"];
                    
                    // Check for single texture for all faces
                    if (texJson.contains("all") && texJson["all"].is_string()) {
                        matDef.textures.all = packPath + "/" + texJson["all"].get<std::string>();
                    }
                    // Or face-specific textures
                    else {
                        if (texJson.contains("top"))
                            matDef.textures.top = packPath + "/" + texJson["top"].get<std::string>();
                        if (texJson.contains("bottom"))
                            matDef.textures.bottom = packPath + "/" + texJson["bottom"].get<std::string>();
                        if (texJson.contains("north"))
                            matDef.textures.north = packPath + "/" + texJson["north"].get<std::string>();
                        if (texJson.contains("south"))
                            matDef.textures.south = packPath + "/" + texJson["south"].get<std::string>();
                        if (texJson.contains("east"))
                            matDef.textures.east = packPath + "/" + texJson["east"].get<std::string>();
                        if (texJson.contains("west"))
                            matDef.textures.west = packPath + "/" + texJson["west"].get<std::string>();
                        
                        // If sides are specified, apply to all horizontal faces
                        if (texJson.contains("sides") && texJson["sides"].is_string()) {
                            std::string sidesPath = packPath + "/" + texJson["sides"].get<std::string>();
                            if (matDef.textures.north.empty()) matDef.textures.north = sidesPath;
                            if (matDef.textures.south.empty()) matDef.textures.south = sidesPath;
                            if (matDef.textures.east.empty()) matDef.textures.east = sidesPath;
                            if (matDef.textures.west.empty()) matDef.textures.west = sidesPath;
                        }
                    }
                }

                // Parse material properties
                if (matJson.contains("materialProperties")) {
                    const auto& propsJson = matJson["materialProperties"];
                    
                    if (propsJson.contains("normalMap"))
                        matDef.materialProps.normalMap = packPath + "/" + propsJson["normalMap"].get<std::string>();
                    if (propsJson.contains("metallicRoughnessMap"))
                        matDef.materialProps.metallicRoughnessMap = packPath + "/" + propsJson["metallicRoughnessMap"].get<std::string>();
                    if (propsJson.contains("aoMap"))
                        matDef.materialProps.aoMap = packPath + "/" + propsJson["aoMap"].get<std::string>();
                    if (propsJson.contains("emissiveMap"))
                        matDef.materialProps.emissiveMap = packPath + "/" + propsJson["emissiveMap"].get<std::string>();
                    
                    matDef.materialProps.metallic = propsJson.value("metallic", 0.0f);
                    matDef.materialProps.roughness = propsJson.value("roughness", 0.5f);
                    matDef.materialProps.ao = propsJson.value("ao", 1.0f);
                    matDef.materialProps.alpha = propsJson.value("alpha", 1.0f);
                    matDef.materialProps.alphaBlend = propsJson.value("alphaBlend", false);
                    matDef.materialProps.alphaCutoffValue = propsJson.value("alphaCutoffValue", 0.5f);
                    
                    if (propsJson.contains("emissive") && propsJson["emissive"].is_array() && propsJson["emissive"].size() == 3) {
                        matDef.materialProps.emissive = glm::vec3(
                            propsJson["emissive"][0].get<float>(),
                            propsJson["emissive"][1].get<float>(),
                            propsJson["emissive"][2].get<float>()
                        );
                    }
                    matDef.materialProps.emissiveStrength = propsJson.value("emissiveStrength", 1.0f);
                }

                matDef.includeInAtlas = matJson.value("includeInAtlas", true);

                // Store definition
                size_t index = materialDefinitions.size();
                materialDefinitions.push_back(matDef);
                voxelTypeToDefIndex[matDef.voxelType] = index;
            }
        }

        LOG_INFO("Parsed material pack manifest: " + name + " v" + version + " by " + author);
        LOG_INFO("  Loaded " + std::to_string(materialDefinitions.size()) + " material definitions");
        return true;

    } catch (const json::exception& e) {
        LOG_ERROR("JSON parsing error: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        LOG_ERROR("Error parsing manifest: " + std::string(e.what()));
        return false;
    }
#else
    // Fallback when JSON not available
    LOG_WARNING("JSON library not available - using minimal fallback");
    
    name = std::filesystem::path(packPath).filename().string();
    version = "1.0.0";
    author = "Fresh Engine";
    description = "Fallback material pack";
    
    // Create basic stone material as example
    VoxelMaterialDefinition stoneDef;
    stoneDef.voxelType = VoxelType::Stone;
    stoneDef.name = "Stone";
    stoneDef.textures.all = packPath + "/textures/stone.png";
    materialDefinitions.push_back(stoneDef);
    voxelTypeToDefIndex[VoxelType::Stone] = 0;
    
    return true;
#endif
}

const VoxelMaterialDefinition* VoxelMaterialPack::getMaterialDefinition(VoxelType type) const
{
    auto it = voxelTypeToDefIndex.find(type);
    if (it != voxelTypeToDefIndex.end()) {
        return &materialDefinitions[it->second];
    }
    return nullptr;
}

std::shared_ptr<Texture> VoxelMaterialPack::getTexture(VoxelType type, const std::string& face)
{
    const auto* matDef = getMaterialDefinition(type);
    if (!matDef) {
        return nullptr;
    }

    // Determine which texture path to use
    std::string texturePath;
    if (matDef->textures.hasAllFaces()) {
        texturePath = matDef->textures.all;
    } else {
        if (face == "top" && !matDef->textures.top.empty())
            texturePath = matDef->textures.top;
        else if (face == "bottom" && !matDef->textures.bottom.empty())
            texturePath = matDef->textures.bottom;
        else if (face == "north" && !matDef->textures.north.empty())
            texturePath = matDef->textures.north;
        else if (face == "south" && !matDef->textures.south.empty())
            texturePath = matDef->textures.south;
        else if (face == "east" && !matDef->textures.east.empty())
            texturePath = matDef->textures.east;
        else if (face == "west" && !matDef->textures.west.empty())
            texturePath = matDef->textures.west;
        else
            return nullptr; // No texture for this face
    }

    if (texturePath.empty()) {
        return nullptr;
    }

    // Check cache
    auto it = textureCache.find(texturePath);
    if (it != textureCache.end()) {
        return it->second;
    }

    // Load texture
    auto& texManager = TextureManager::getInstance();
    auto texture = texManager.loadTexture(texturePath);
    if (texture) {
        textureCache[texturePath] = texture;
    }

    return texture;
}

bool VoxelMaterialPack::overridesVoxelType(VoxelType type) const
{
    return voxelTypeToDefIndex.find(type) != voxelTypeToDefIndex.end();
}

std::shared_ptr<Texture> VoxelMaterialPack::generateTextureAtlas(int atlasSize, int tileSize)
{
    LOG_INFO("Generating texture atlas for pack: " + name);
    LOG_INFO("  Atlas size: " + std::to_string(atlasSize) + "x" + std::to_string(atlasSize));
    LOG_INFO("  Tile size: " + std::to_string(tileSize) + "x" + std::to_string(tileSize));

    // TODO: Implement actual texture atlas generation
    // This is a placeholder that would need proper image manipulation
    LOG_WARNING("Texture atlas generation not yet implemented");
    
    return nullptr;
}

std::vector<float> VoxelMaterialPack::getAtlasUVs(VoxelType type, const std::string& face) const
{
    std::string key = voxelTypeToString(type) + ":" + face;
    auto it = atlasMapping.find(key);
    if (it != atlasMapping.end()) {
        const auto& entry = it->second;
        return {entry.uvMin[0], entry.uvMin[1], entry.uvMax[0], entry.uvMax[1]};
    }
    return {}; // Empty if not in atlas
}

// VoxelMaterialPackManager implementation

VoxelMaterialPackManager& VoxelMaterialPackManager::getInstance()
{
    static VoxelMaterialPackManager instance;
    return instance;
}

void VoxelMaterialPackManager::initialize(const std::string& packDir)
{
    packDirectory = packDir;
    
    LOG_INFO("=== Voxel Material Pack Manager ===");
    LOG_INFO("Pack directory: " + packDirectory);
    
    scanAndLoadPacks();
}

void VoxelMaterialPackManager::shutdown()
{
    LOG_INFO("Shutting down Voxel Material Pack Manager...");
    
    for (auto& pack : loadedPacks) {
        pack->unload();
    }
    loadedPacks.clear();
}

void VoxelMaterialPackManager::scanAndLoadPacks()
{
    if (!std::filesystem::exists(packDirectory)) {
        LOG_INFO("Pack directory does not exist, creating: " + packDirectory);
        std::filesystem::create_directories(packDirectory);
        return;
    }

    LOG_INFO("Scanning for voxel material packs...");

    int packsFound = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(packDirectory)) {
            if (entry.is_directory()) {
                std::string packPath = entry.path().string();
                
                // Check if it has a voxel materials manifest
                if (std::filesystem::exists(packPath + "/voxel_materials.json") ||
                    std::filesystem::exists(packPath + "/materials.json")) {
                    if (loadPack(packPath)) {
                        packsFound++;
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Error scanning pack directory: " + std::string(e.what()));
    }

    sortPacksByPriority();
    
    LOG_INFO("Found and loaded " + std::to_string(packsFound) + " voxel material packs");
}

bool VoxelMaterialPackManager::loadPack(const std::string& packPath)
{
    auto pack = std::make_shared<VoxelMaterialPack>(packPath);
    if (pack->load()) {
        loadedPacks.push_back(pack);
        sortPacksByPriority();
        return true;
    }
    return false;
}

void VoxelMaterialPackManager::unloadPack(const std::string& packName)
{
    auto it = std::remove_if(loadedPacks.begin(), loadedPacks.end(),
                             [&packName](const std::shared_ptr<VoxelMaterialPack>& pack) {
                                 return pack->getName() == packName;
                             });

    if (it != loadedPacks.end()) {
        loadedPacks.erase(it, loadedPacks.end());
        LOG_INFO("Unloaded voxel material pack: " + packName);
    }
}

void VoxelMaterialPackManager::sortPacksByPriority()
{
    std::sort(loadedPacks.begin(), loadedPacks.end(),
              [](const std::shared_ptr<VoxelMaterialPack>& a,
                 const std::shared_ptr<VoxelMaterialPack>& b) {
                  return a->getPriority() > b->getPriority(); // Higher priority first
              });
}

const VoxelMaterialDefinition* VoxelMaterialPackManager::getMaterialDefinition(VoxelType type) const
{
    // Return from highest priority pack that has this type
    for (const auto& pack : loadedPacks) {
        const auto* matDef = pack->getMaterialDefinition(type);
        if (matDef) {
            return matDef;
        }
    }
    return nullptr;
}

std::shared_ptr<Texture> VoxelMaterialPackManager::getTexture(VoxelType type, const std::string& face)
{
    // Return from highest priority pack that has this type
    for (auto& pack : loadedPacks) {
        if (pack->overridesVoxelType(type)) {
            return pack->getTexture(type, face);
        }
    }
    return nullptr;
}

bool VoxelMaterialPackManager::createPackTemplate(const std::string& outputPath)
{
    LOG_INFO("=== Creating Voxel Material Pack Template ===");
    LOG_INFO("Output path: " + outputPath);

    // Create directory structure
    std::filesystem::create_directories(outputPath);
    std::filesystem::create_directories(outputPath + "/textures");

    // Create README
    std::string readmePath = outputPath + "/README.md";
    std::ofstream readme(readmePath);
    if (readme.is_open()) {
        readme << "# Voxel Material Pack Template\n\n";
        readme << "This template helps you create custom voxel block textures and materials.\n\n";
        
        readme << "## Directory Structure\n\n";
        readme << "```\n";
        readme << "MyVoxelPack/\n";
        readme << "├── voxel_materials.json  # Material definitions (REQUIRED)\n";
        readme << "├── README.md             # This file\n";
        readme << "└── textures/             # Texture files\n";
        readme << "    ├── stone.png\n";
        readme << "    ├── grass_top.png\n";
        readme << "    ├── grass_side.png\n";
        readme << "    └── ...\n";
        readme << "```\n\n";
        
        readme << "## Manifest File (voxel_materials.json)\n\n";
        readme << "The manifest defines which voxel types to customize and their textures/materials.\n\n";
        
        readme << "### Pack Metadata\n";
        readme << "- `name`: Pack name (unique identifier)\n";
        readme << "- `version`: Version number (e.g., \"1.0.0\")\n";
        readme << "- `author`: Creator name\n";
        readme << "- `description`: Brief description\n";
        readme << "- `priority`: Priority for pack ordering (default: 0, higher = more important)\n\n";
        
        readme << "### Material Definitions\n\n";
        readme << "Each entry in the `materials` array defines textures for a voxel type:\n\n";
        
        readme << "#### Required Fields\n";
        readme << "- `voxelType`: Voxel type name (Stone, Dirt, Grass, Sand, Water, Wood, Leaves, etc.)\n";
        readme << "- `name`: Display name for this material\n";
        readme << "- `textures`: Texture paths\n\n";
        
        readme << "#### Texture Options\n\n";
        readme << "**Single texture for all faces:**\n";
        readme << "```json\n";
        readme << "\"textures\": {\n";
        readme << "  \"all\": \"textures/stone.png\"\n";
        readme << "}\n";
        readme << "```\n\n";
        
        readme << "**Face-specific textures (like grass block):**\n";
        readme << "```json\n";
        readme << "\"textures\": {\n";
        readme << "  \"top\": \"textures/grass_top.png\",\n";
        readme << "  \"bottom\": \"textures/dirt.png\",\n";
        readme << "  \"sides\": \"textures/grass_side.png\"\n";
        readme << "}\n";
        readme << "```\n\n";
        
        readme << "Or specify each face individually: `north`, `south`, `east`, `west`\n\n";
        
        readme << "#### Optional Material Properties\n\n";
        readme << "For PBR materials, add `materialProperties`:\n\n";
        readme << "```json\n";
        readme << "\"materialProperties\": {\n";
        readme << "  \"normalMap\": \"textures/stone_normal.png\",\n";
        readme << "  \"metallicRoughnessMap\": \"textures/stone_mr.png\",\n";
        readme << "  \"metallic\": 0.0,\n";
        readme << "  \"roughness\": 0.8,\n";
        readme << "  \"alpha\": 1.0,\n";
        readme << "  \"alphaBlend\": false\n";
        readme << "}\n";
        readme << "```\n\n";
        
        readme << "## Supported Voxel Types\n\n";
        readme << "- `Air` - Empty space\n";
        readme << "- `Stone` - Stone blocks\n";
        readme << "- `Dirt` - Dirt blocks\n";
        readme << "- `Grass` - Grass blocks (supports face-specific textures)\n";
        readme << "- `Sand` - Sand blocks\n";
        readme << "- `Water` - Water blocks (transparent)\n";
        readme << "- `Wood` - Wood/log blocks\n";
        readme << "- `Leaves` - Leaf blocks (transparent)\n";
        readme << "- `Bedrock` - Bedrock blocks\n";
        readme << "- `Snow` - Snow blocks\n";
        readme << "- `Ice` - Ice blocks (transparent)\n";
        readme << "- `Cobblestone` - Cobblestone blocks\n";
        readme << "- `Planks` - Wooden plank blocks\n";
        readme << "- `Glass` - Glass blocks (transparent)\n\n";
        
        readme << "## Texture Guidelines\n\n";
        readme << "- **Resolution**: 16x16, 32x32, or 64x64 pixels recommended\n";
        readme << "- **Format**: PNG (supports transparency), JPG, or TGA\n";
        readme << "- **Tileable**: Textures should tile seamlessly\n";
        readme << "- **Consistent style**: Match existing voxel aesthetic or create unique look\n";
        readme << "- **Transparency**: Use PNG alpha channel for transparent blocks\n\n";
        
        readme << "## Installation\n\n";
        readme << "1. Create your textures using pixel art tools (Aseprite, GIMP, Photoshop, etc.)\n";
        readme << "2. Place textures in the `textures/` directory\n";
        readme << "3. Edit `voxel_materials.json` to define your materials\n";
        readme << "4. Copy the pack folder to the engine's materials directory\n";
        readme << "5. Restart the engine - pack will load automatically\n\n";
        
        readme << "## Examples\n\n";
        readme << "See the included `voxel_materials.json` for complete examples.\n\n";
        
        readme.close();
        LOG_INFO("  Created README.md");
    }

    // Create manifest template
    std::string manifestPath = outputPath + "/voxel_materials.json";
    std::ofstream manifest(manifestPath);
    if (!manifest.is_open()) {
        LOG_ERROR("Failed to create manifest file");
        return false;
    }

    manifest << "{\n";
    manifest << "  \"name\": \"MyVoxelMaterialPack\",\n";
    manifest << "  \"version\": \"1.0.0\",\n";
    manifest << "  \"author\": \"Your Name\",\n";
    manifest << "  \"description\": \"Custom voxel block textures and materials\",\n";
    manifest << "  \"priority\": 0,\n";
    manifest << "  \"materials\": [\n";
    manifest << "    {\n";
    manifest << "      \"voxelType\": \"Stone\",\n";
    manifest << "      \"name\": \"Custom Stone\",\n";
    manifest << "      \"textures\": {\n";
    manifest << "        \"all\": \"textures/stone.png\"\n";
    manifest << "      },\n";
    manifest << "      \"materialProperties\": {\n";
    manifest << "        \"roughness\": 0.8,\n";
    manifest << "        \"metallic\": 0.0\n";
    manifest << "      },\n";
    manifest << "      \"includeInAtlas\": true\n";
    manifest << "    },\n";
    manifest << "    {\n";
    manifest << "      \"voxelType\": \"Grass\",\n";
    manifest << "      \"name\": \"Custom Grass\",\n";
    manifest << "      \"textures\": {\n";
    manifest << "        \"top\": \"textures/grass_top.png\",\n";
    manifest << "        \"bottom\": \"textures/dirt.png\",\n";
    manifest << "        \"sides\": \"textures/grass_side.png\"\n";
    manifest << "      },\n";
    manifest << "      \"includeInAtlas\": true\n";
    manifest << "    },\n";
    manifest << "    {\n";
    manifest << "      \"voxelType\": \"Dirt\",\n";
    manifest << "      \"name\": \"Custom Dirt\",\n";
    manifest << "      \"textures\": {\n";
    manifest << "        \"all\": \"textures/dirt.png\"\n";
    manifest << "      },\n";
    manifest << "      \"includeInAtlas\": true\n";
    manifest << "    },\n";
    manifest << "    {\n";
    manifest << "      \"voxelType\": \"Water\",\n";
    manifest << "      \"name\": \"Custom Water\",\n";
    manifest << "      \"textures\": {\n";
    manifest << "        \"all\": \"textures/water.png\"\n";
    manifest << "      },\n";
    manifest << "      \"materialProperties\": {\n";
    manifest << "        \"alpha\": 0.6,\n";
    manifest << "        \"alphaBlend\": true\n";
    manifest << "      },\n";
    manifest << "      \"includeInAtlas\": true\n";
    manifest << "    }\n";
    manifest << "  ]\n";
    manifest << "}\n";
    manifest.close();

    LOG_INFO("  Created voxel_materials.json template");
    LOG_INFO("\n=== Voxel Material Pack Template Created Successfully ===");
    LOG_INFO("Location: " + outputPath);
    LOG_INFO("\nNext steps:");
    LOG_INFO("1. Read the README.md for detailed instructions");
    LOG_INFO("2. Create your block textures (16x16, 32x32, or 64x64 PNG files)");
    LOG_INFO("3. Place textures in the textures/ directory");
    LOG_INFO("4. Edit voxel_materials.json to define your materials");
    LOG_INFO("5. Install the pack by copying to the materials directory");

    return true;
}

bool VoxelMaterialPackManager::validatePack(const std::string& packPath) const
{
    // Check if directory exists
    if (!std::filesystem::exists(packPath)) {
        LOG_ERROR("Pack directory does not exist: " + packPath);
        return false;
    }

    // Check for manifest
    if (!std::filesystem::exists(packPath + "/voxel_materials.json") &&
        !std::filesystem::exists(packPath + "/materials.json")) {
        LOG_ERROR("Manifest file missing in: " + packPath);
        return false;
    }

    return true;
}

void VoxelMaterialPackManager::printStats() const
{
    LOG_INFO("\n=== Voxel Material Pack Statistics ===");
    LOG_INFO("Loaded packs: " + std::to_string(loadedPacks.size()));

    for (const auto& pack : loadedPacks) {
        LOG_INFO("  " + pack->getName() + " v" + pack->getVersion() +
                 " (priority: " + std::to_string(pack->getPriority()) + ")");
        LOG_INFO("    Materials: " + std::to_string(pack->getMaterialDefinitions().size()));
    }
}

} // namespace fresh
