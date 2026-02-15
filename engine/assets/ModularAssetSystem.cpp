#include "assets/ModularAssetSystem.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>

#ifdef FRESH_JSON_AVAILABLE
#include <nlohmann/json.hpp>
using json = nlohmann::json;
#endif

#include "core/ResourceManager.h"
#include "voxel/VoxelWorld.h"

namespace fresh
{

// AssetPack implementation
AssetPack::AssetPack(const std::string& packPath) : packPath(packPath) {}

AssetPack::~AssetPack()
{
    unload();
}

bool AssetPack::load()
{
    if (loaded)
        return true;

    std::cout << "Loading asset pack from: " << packPath << std::endl;

    // Look for manifest file
    std::string manifestPath = packPath + "/manifest.json";
    if (!std::filesystem::exists(manifestPath)) {
        std::cerr << "Manifest not found: " << manifestPath << std::endl;
        return false;
    }

    if (!parseManifest(manifestPath)) {
        std::cerr << "Failed to parse manifest: " << manifestPath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Loaded asset pack: " << name << " v" << version << std::endl;
    std::cout << "  Assets: " << assets.size() << std::endl;
    return true;
}

void AssetPack::unload()
{
    if (!loaded)
        return;

    std::cout << "Unloading asset pack: " << name << std::endl;
    assets.clear();
    loaded = false;
}

std::vector<AssetMetadata> AssetPack::getAssetsByBiome(BiomeType biome) const
{
    std::vector<AssetMetadata> result;
    for (const auto& asset : assets) {
        if (std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(), biome) !=
                asset.allowedBiomes.end() ||
            std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(), BiomeType::Any) !=
                asset.allowedBiomes.end()) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetMetadata> AssetPack::getAssetsByTag(const std::string& tag) const
{
    std::vector<AssetMetadata> result;
    for (const auto& asset : assets) {
        if (std::find(asset.tags.begin(), asset.tags.end(), tag) != asset.tags.end()) {
            result.push_back(asset);
        }
    }
    return result;
}

std::vector<AssetMetadata> AssetPack::getAssetsByRule(PlacementRule rule) const
{
    std::vector<AssetMetadata> result;
    for (const auto& asset : assets) {
        if (asset.placementRule == rule) {
            result.push_back(asset);
        }
    }
    return result;
}

bool AssetPack::parseManifest(const std::string& manifestPath)
{
#ifdef FRESH_JSON_AVAILABLE
    try {
        // Read JSON file
        std::ifstream file(manifestPath);
        if (!file.is_open()) {
            std::cerr << "Failed to open manifest file: " << manifestPath << std::endl;
            return false;
        }

        json manifestJson;
        file >> manifestJson;
        file.close();

        // Parse pack metadata
        name = manifestJson.value("name", std::filesystem::path(packPath).filename().string());
        version = manifestJson.value("version", "1.0.0");
        author = manifestJson.value("author", "Unknown");

        // Parse assets array
        if (manifestJson.contains("assets") && manifestJson["assets"].is_array()) {
            for (const auto& assetJson : manifestJson["assets"]) {
                AssetMetadata asset;
                
                // Required fields
                asset.name = assetJson.value("name", "");
                asset.description = assetJson.value("description", "");
                asset.modelPath = packPath + "/" + assetJson.value("modelPath", "");
                asset.texturePath = packPath + "/" + assetJson.value("texturePath", "");

                // Parse biomes
                if (assetJson.contains("biomes") && assetJson["biomes"].is_array()) {
                    for (const auto& biomeStr : assetJson["biomes"]) {
                        std::string biome = biomeStr.get<std::string>();
                        if (biome == "Any") asset.allowedBiomes.push_back(BiomeType::Any);
                        else if (biome == "Forest") asset.allowedBiomes.push_back(BiomeType::Forest);
                        else if (biome == "Plains") asset.allowedBiomes.push_back(BiomeType::Plains);
                        else if (biome == "Desert") asset.allowedBiomes.push_back(BiomeType::Desert);
                        else if (biome == "Mountains") asset.allowedBiomes.push_back(BiomeType::Mountains);
                        else if (biome == "Swamp") asset.allowedBiomes.push_back(BiomeType::Swamp);
                        else if (biome == "Tundra") asset.allowedBiomes.push_back(BiomeType::Tundra);
                        else if (biome == "Ocean") asset.allowedBiomes.push_back(BiomeType::Ocean);
                        else if (biome == "Cave") asset.allowedBiomes.push_back(BiomeType::Cave);
                    }
                }
                if (asset.allowedBiomes.empty()) {
                    asset.allowedBiomes.push_back(BiomeType::Any);
                }

                // Parse placement rule
                std::string ruleStr = assetJson.value("placementRule", "Random");
                if (ruleStr == "Clustered") asset.placementRule = PlacementRule::Clustered;
                else if (ruleStr == "Grid") asset.placementRule = PlacementRule::Grid;
                else if (ruleStr == "NearWater") asset.placementRule = PlacementRule::NearWater;
                else if (ruleStr == "OnHills") asset.placementRule = PlacementRule::OnHills;
                else if (ruleStr == "InCaves") asset.placementRule = PlacementRule::InCaves;
                else if (ruleStr == "RiverSide") asset.placementRule = PlacementRule::RiverSide;
                else asset.placementRule = PlacementRule::Random;

                // Parse placement parameters
                asset.spawnChance = assetJson.value("spawnChance", 0.1f);
                asset.minDistance = assetJson.value("minDistance", 10.0f);
                asset.minGroupSize = assetJson.value("minGroupSize", 1);
                asset.maxGroupSize = assetJson.value("maxGroupSize", 5);

                // Parse size
                if (assetJson.contains("size") && assetJson["size"].is_array() && assetJson["size"].size() == 3) {
                    asset.size = glm::vec3(
                        assetJson["size"][0].get<float>(),
                        assetJson["size"][1].get<float>(),
                        assetJson["size"][2].get<float>()
                    );
                } else {
                    asset.size = glm::vec3(1.0f);
                }

                // Parse offset
                if (assetJson.contains("offset") && assetJson["offset"].is_array() && assetJson["offset"].size() == 3) {
                    asset.offset = glm::vec3(
                        assetJson["offset"][0].get<float>(),
                        assetJson["offset"][1].get<float>(),
                        assetJson["offset"][2].get<float>()
                    );
                }

                // Parse rotation settings
                asset.randomRotation = assetJson.value("randomRotation", true);
                asset.rotationStep = assetJson.value("rotationStep", 90.0f);

                // Parse world generation settings
                asset.placeOnSurface = assetJson.value("placeOnSurface", true);
                asset.embedInTerrain = assetJson.value("embedInTerrain", false);
                asset.embedDepth = assetJson.value("embedDepth", 0.0f);

                // Parse tags
                if (assetJson.contains("tags") && assetJson["tags"].is_array()) {
                    for (const auto& tag : assetJson["tags"]) {
                        asset.tags.push_back(tag.get<std::string>());
                    }
                }

                // Parse custom properties
                if (assetJson.contains("customProperties") && assetJson["customProperties"].is_object()) {
                    for (auto& [key, value] : assetJson["customProperties"].items()) {
                        if (value.is_string()) {
                            asset.customProperties[key] = value.get<std::string>();
                        }
                    }
                }

                assets.push_back(asset);
            }
        }

        std::cout << "Parsed manifest: " << name << " v" << version << " by " << author << std::endl;
        std::cout << "  Loaded " << assets.size() << " asset definitions" << std::endl;
        return true;

    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing manifest: " << e.what() << std::endl;
        return false;
    }
#else
    // Fallback implementation when JSON library is not available
    std::cout << "JSON library not available - using fallback asset loading" << std::endl;
    
    name = std::filesystem::path(packPath).filename().string();
    version = "1.0.0";
    author = "Fresh Engine";

    // Example tree asset
    AssetMetadata tree;
    tree.name = "Oak_Tree";
    tree.description = "Large oak tree";
    tree.modelPath = packPath + "/models/oak_tree.obj";
    tree.texturePath = packPath + "/textures/oak_tree.png";
    tree.allowedBiomes = {BiomeType::Forest, BiomeType::Plains};
    tree.placementRule = PlacementRule::Clustered;
    tree.spawnChance = 0.15f;
    tree.minDistance = 8.0f;
    tree.minGroupSize = 2;
    tree.maxGroupSize = 8;
    tree.size = glm::vec3(3.0f, 12.0f, 3.0f);
    tree.randomRotation = true;
    tree.placeOnSurface = true;
    tree.tags = {"tree", "vegetation", "natural"};
    assets.push_back(tree);

    // Example rock asset
    AssetMetadata rock;
    rock.name = "Boulder";
    rock.description = "Large boulder";
    rock.modelPath = packPath + "/models/boulder.obj";
    rock.texturePath = packPath + "/textures/boulder.png";
    rock.allowedBiomes = {BiomeType::Mountains, BiomeType::Plains};
    rock.placementRule = PlacementRule::Random;
    rock.spawnChance = 0.08f;
    rock.minDistance = 15.0f;
    rock.size = glm::vec3(2.5f, 2.0f, 2.5f);
    rock.randomRotation = true;
    rock.embedInTerrain = true;
    rock.embedDepth = 0.3f;
    rock.tags = {"rock", "natural", "obstacle"};
    assets.push_back(rock);

    return true;
#endif
}

// ModularAssetSystem implementation
ModularAssetSystem& ModularAssetSystem::getInstance()
{
    static ModularAssetSystem instance;
    return instance;
}

void ModularAssetSystem::initialize(const std::string& assetDir)
{
    assetDirectory = assetDir;

    std::cout << "=== Modular Asset System ===" << std::endl;
    std::cout << "Asset directory: " << assetDirectory << std::endl;

    // Register default placement functions
    registerPlacementFunction(
        PlacementRule::Random,
        [this](const AssetMetadata& asset, VoxelWorld* world, int cx, int cz, uint32_t seed) {
            return placeRandomly(asset, world, cx, cz, seed);
        });

    registerPlacementFunction(
        PlacementRule::Clustered,
        [this](const AssetMetadata& asset, VoxelWorld* world, int cx, int cz, uint32_t seed) {
            return placeClustered(asset, world, cx, cz, seed);
        });

    registerPlacementFunction(
        PlacementRule::Grid,
        [this](const AssetMetadata& asset, VoxelWorld* world, int cx, int cz, uint32_t seed) {
            return placeGrid(asset, world, cx, cz, seed);
        });

    // Scan for asset packs
    scanAndLoadAssetPacks();
}

void ModularAssetSystem::shutdown()
{
    std::cout << "Shutting down Modular Asset System..." << std::endl;

    for (auto& pack : loadedPacks) {
        pack->unload();
    }
    loadedPacks.clear();
    placedAssetPositions.clear();
}

void ModularAssetSystem::scanAndLoadAssetPacks()
{
    if (!std::filesystem::exists(assetDirectory)) {
        std::cout << "Asset directory does not exist, creating: " << assetDirectory << std::endl;
        std::filesystem::create_directories(assetDirectory);
        return;
    }

    std::cout << "Scanning for asset packs..." << std::endl;

    int packsFound = 0;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(assetDirectory)) {
            if (entry.is_directory()) {
                std::string packPath = entry.path().string();
                if (loadAssetPack(packPath)) {
                    packsFound++;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error scanning asset directory: " << e.what() << std::endl;
    }

    std::cout << "Found and loaded " << packsFound << " asset packs" << std::endl;
}

bool ModularAssetSystem::loadAssetPack(const std::string& packPath)
{
    auto pack = std::make_shared<AssetPack>(packPath);
    if (pack->load()) {
        loadedPacks.push_back(pack);
        return true;
    }
    return false;
}

void ModularAssetSystem::unloadAssetPack(const std::string& packName)
{
    auto it = std::remove_if(loadedPacks.begin(), loadedPacks.end(),
                             [&packName](const std::shared_ptr<AssetPack>& pack) {
                                 return pack->getName() == packName;
                             });

    if (it != loadedPacks.end()) {
        loadedPacks.erase(it, loadedPacks.end());
        std::cout << "Unloaded asset pack: " << packName << std::endl;
    }
}

AssetPack* ModularAssetSystem::getAssetPack(const std::string& name)
{
    for (auto& pack : loadedPacks) {
        if (pack->getName() == name) {
            return pack.get();
        }
    }
    return nullptr;
}

std::vector<AssetMetadata> ModularAssetSystem::queryAssets(BiomeType biome, const std::string& tag,
                                                           PlacementRule rule) const
{
    std::vector<AssetMetadata> result;

    for (const auto& pack : loadedPacks) {
        const auto& packAssets = pack->getAssets();

        for (const auto& asset : packAssets) {
            bool matches = true;

            // Filter by biome
            if (biome != BiomeType::Any) {
                bool biomeMatch = std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(),
                                            biome) != asset.allowedBiomes.end();
                bool anyBiome = std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(),
                                          BiomeType::Any) != asset.allowedBiomes.end();
                matches = matches && (biomeMatch || anyBiome);
            }

            // Filter by tag
            if (!tag.empty()) {
                bool tagMatch =
                    std::find(asset.tags.begin(), asset.tags.end(), tag) != asset.tags.end();
                matches = matches && tagMatch;
            }

            // Filter by placement rule
            if (rule != PlacementRule::Random) {
                matches = matches && (asset.placementRule == rule);
            }

            if (matches) {
                result.push_back(asset);
            }
        }
    }

    return result;
}

std::vector<AssetInstance> ModularAssetSystem::generateAssetsForChunk(VoxelWorld* world, int chunkX,
                                                                      int chunkZ, uint32_t seed)
{
    std::vector<AssetInstance> instances;
    if (!world)
        return instances;

    // Create RNG with chunk-specific seed
    std::mt19937 rng(seed ^ (chunkX * 73856093) ^ (chunkZ * 19349663));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Get all available assets
    auto allAssets = queryAssets();

    for (const auto& asset : allAssets) {
        // Check spawn chance
        if (dist(rng) > asset.spawnChance) {
            continue;
        }

        // Use registered placement function
        auto it = placementFunctions.find(asset.placementRule);
        if (it != placementFunctions.end()) {
            auto assetInstances = it->second(asset, world, chunkX, chunkZ, seed);
            instances.insert(instances.end(), assetInstances.begin(), assetInstances.end());
        }
    }

    return instances;
}

void ModularAssetSystem::placeAssetsInWorld(const std::vector<AssetInstance>& instances,
                                            VoxelWorld* world)
{
    if (!world)
        return;

    for (const auto& instance : instances) {
        std::cout << "Placing asset: " << instance.assetName << " at (" << instance.position.x
                  << ", " << instance.position.y << ", " << instance.position.z << ")" << std::endl;

        // Get the asset metadata to determine size
        AssetMetadata* assetMeta = nullptr;
        for (const auto& pack : loadedPacks) {
            for (const auto& asset : pack->getAssets()) {
                if (asset.name == instance.assetName) {
                    assetMeta = const_cast<AssetMetadata*>(&asset);
                    break;
                }
            }
            if (assetMeta)
                break;
        }

        if (!assetMeta) {
            std::cerr << "Asset metadata not found for: " << instance.assetName << std::endl;
            continue;
        }

        // Simple voxel-based representation
        // For now, we'll place a solid block of voxels matching the asset's size
        // In a full implementation, this would load actual 3D models
        
        // Determine voxel type based on asset tags
        VoxelType voxelType = VoxelType::Wood; // Default
        if (std::find(assetMeta->tags.begin(), assetMeta->tags.end(), "rock") != assetMeta->tags.end()) {
            voxelType = VoxelType::Stone;
        } else if (std::find(assetMeta->tags.begin(), assetMeta->tags.end(), "tree") != assetMeta->tags.end() ||
                   std::find(assetMeta->tags.begin(), assetMeta->tags.end(), "vegetation") != assetMeta->tags.end()) {
            voxelType = VoxelType::Wood;
        }

        // Calculate placement bounds
        int startX = static_cast<int>(instance.position.x - assetMeta->size.x / 2.0f);
        int startY = static_cast<int>(instance.position.y);
        int startZ = static_cast<int>(instance.position.z - assetMeta->size.z / 2.0f);
        
        int endX = static_cast<int>(instance.position.x + assetMeta->size.x / 2.0f);
        int endY = static_cast<int>(instance.position.y + assetMeta->size.y);
        int endZ = static_cast<int>(instance.position.z + assetMeta->size.z / 2.0f);

        // Apply embed depth if configured
        if (assetMeta->embedInTerrain && assetMeta->embedDepth > 0.0f) {
            startY -= static_cast<int>(assetMeta->embedDepth);
        }

        // Place voxels
        for (int x = startX; x < endX; ++x) {
            for (int y = startY; y < endY; ++y) {
                for (int z = startZ; z < endZ; ++z) {
                    // Simple shape - could be enhanced with more complex patterns
                    // For trees, create a trunk and leaves pattern
                    if (voxelType == VoxelType::Wood && std::find(assetMeta->tags.begin(), assetMeta->tags.end(), "tree") != assetMeta->tags.end()) {
                        // Trunk in the center
                        int centerX = static_cast<int>(instance.position.x);
                        int centerZ = static_cast<int>(instance.position.z);
                        
                        if (x == centerX && z == centerZ && y < startY + assetMeta->size.y * 0.7f) {
                            // Trunk
                            WorldPos pos(x, y, z);
                            world->setVoxel(pos, Voxel(VoxelType::Wood));
                        } else if (y >= startY + assetMeta->size.y * 0.5f) {
                            // Leaves in upper portion
                            float distFromCenter = static_cast<float>(std::sqrt((x - centerX) * (x - centerX) + (z - centerZ) * (z - centerZ)));
                            if (distFromCenter < assetMeta->size.x / 2.0f) {
                                WorldPos pos(x, y, z);
                                world->setVoxel(pos, Voxel(VoxelType::Leaves));
                            }
                        }
                    } else {
                        // Simple solid block for other assets
                        WorldPos pos(x, y, z);
                        world->setVoxel(pos, Voxel(voxelType));
                    }
                }
            }
        }

        // Track placement
        glm::vec2 pos2D(instance.position.x, instance.position.z);
        placedAssetPositions[instance.assetName].push_back(pos2D);
    }
}

bool ModularAssetSystem::createAssetPackTemplate(const std::string& outputPath)
{
    std::cout << "=== Creating Asset Pack Template ===" << std::endl;
    std::cout << "Output path: " << outputPath << std::endl;

    // Create directory structure
    std::filesystem::create_directories(outputPath);
    std::filesystem::create_directories(outputPath + "/models");
    std::filesystem::create_directories(outputPath + "/textures");

    // Create README file with structure documentation
    std::string readmePath = outputPath + "/README.md";
    std::ofstream readme(readmePath);
    if (readme.is_open()) {
        readme << "# Asset Pack Template\n\n";
        readme << "This is a template for creating custom asset packs for the Fresh Voxel Engine.\n\n";
        
        readme << "## Directory Structure\n\n";
        readme << "```\n";
        readme << "YourAssetPack/\n";
        readme << "├── manifest.json       # Asset pack configuration (REQUIRED)\n";
        readme << "├── README.md           # Documentation (optional)\n";
        readme << "├── models/             # 3D model files (.obj, .fbx, .gltf)\n";
        readme << "│   └── tree.obj\n";
        readme << "└── textures/           # Texture files (.png, .jpg)\n";
        readme << "    └── tree.png\n";
        readme << "```\n\n";
        
        readme << "## Manifest File (manifest.json)\n\n";
        readme << "The manifest.json file is **REQUIRED** and defines your asset pack metadata and asset definitions.\n\n";
        
        readme << "### Pack Metadata\n";
        readme << "- `name`: (string) Unique name for your asset pack\n";
        readme << "- `version`: (string) Version number (e.g., \"1.0.0\")\n";
        readme << "- `author`: (string) Pack creator name\n";
        readme << "- `description`: (string) Brief description of the pack\n\n";
        
        readme << "### Asset Definitions\n";
        readme << "Each asset in the `assets` array can have the following properties:\n\n";
        
        readme << "#### Required Fields\n";
        readme << "- `name`: (string) Unique identifier for the asset\n";
        readme << "- `modelPath`: (string) Path to model file relative to pack root\n";
        readme << "- `texturePath`: (string) Path to texture file relative to pack root\n\n";
        
        readme << "#### Optional Fields\n";
        readme << "- `description`: (string) Asset description\n";
        readme << "- `biomes`: (array) Where asset can spawn. Options:\n";
        readme << "  - `\"Any\"` - Can spawn in any biome (default)\n";
        readme << "  - `\"Forest\"` - Forest biomes\n";
        readme << "  - `\"Plains\"` - Plains biomes\n";
        readme << "  - `\"Desert\"` - Desert biomes\n";
        readme << "  - `\"Mountains\"` - Mountain biomes\n";
        readme << "  - `\"Swamp\"` - Swamp biomes\n";
        readme << "  - `\"Tundra\"` - Tundra biomes\n";
        readme << "  - `\"Ocean\"` - Ocean biomes\n";
        readme << "  - `\"Cave\"` - Cave/underground biomes\n";
        readme << "- `placementRule`: (string) How assets are placed. Options:\n";
        readme << "  - `\"Random\"` - Random placement (default)\n";
        readme << "  - `\"Clustered\"` - Grouped together\n";
        readme << "  - `\"Grid\"` - Regular grid pattern\n";
        readme << "  - `\"NearWater\"` - Near water sources\n";
        readme << "  - `\"OnHills\"` - On elevated terrain\n";
        readme << "  - `\"InCaves\"` - Underground caves\n";
        readme << "  - `\"RiverSide\"` - Along rivers\n";
        readme << "- `spawnChance`: (float) 0.0-1.0, probability of spawning (default: 0.1)\n";
        readme << "- `minDistance`: (float) Minimum distance between instances (default: 10.0)\n";
        readme << "- `minGroupSize`: (int) Minimum group size for clustered placement (default: 1)\n";
        readme << "- `maxGroupSize`: (int) Maximum group size for clustered placement (default: 5)\n";
        readme << "- `size`: (array) [width, height, depth] in voxels (default: [1.0, 1.0, 1.0])\n";
        readme << "- `offset`: (array) [x, y, z] position offset (default: [0.0, 0.0, 0.0])\n";
        readme << "- `randomRotation`: (bool) Random Y-axis rotation (default: true)\n";
        readme << "- `rotationStep`: (float) Rotation angle step in degrees (default: 90.0)\n";
        readme << "- `placeOnSurface`: (bool) Place on terrain surface (default: true)\n";
        readme << "- `embedInTerrain`: (bool) Partially sink into terrain (default: false)\n";
        readme << "- `embedDepth`: (float) How deep to embed if enabled (default: 0.0)\n";
        readme << "- `tags`: (array of strings) Tags for filtering/categorization\n";
        readme << "- `customProperties`: (object) Custom key-value pairs for game-specific data\n\n";
        
        readme << "## Example Manifest\n\n";
        readme << "See the included `manifest.json` for a complete example.\n\n";
        
        readme << "## Installation\n\n";
        readme << "1. Place your asset pack folder in the engine's assets directory\n";
        readme << "2. The engine will automatically discover and load it on startup\n";
        readme << "3. Check the console for loading messages and any errors\n\n";
        
        readme << "## Supported File Formats\n\n";
        readme << "- **Models**: .obj, .fbx, .gltf, .glb, .dae\n";
        readme << "- **Textures**: .png, .jpg, .jpeg, .bmp, .tga, .dds\n\n";
        
        readme << "## Tips\n\n";
        readme << "- Start with low spawn chances (0.05-0.15) to avoid overcrowding\n";
        readme << "- Use `minDistance` to prevent overlapping assets\n";
        readme << "- Test different biomes and placement rules\n";
        readme << "- Use tags for easy filtering and organization\n";
        readme << "- Keep model poly count reasonable for performance\n\n";
        
        readme << "## Troubleshooting\n\n";
        readme << "- **Pack not loading**: Check that manifest.json is valid JSON\n";
        readme << "- **Assets not appearing**: Verify file paths are correct and relative to pack root\n";
        readme << "- **Performance issues**: Reduce spawn chances or increase min distances\n";
        readme << "- **Validation errors**: Check console output for specific error messages\n\n";
        
        readme.close();
        std::cout << "  Created README.md with documentation" << std::endl;
    }

    // Create comprehensive manifest template
    std::string manifestPath = outputPath + "/manifest.json";
    std::ofstream manifest(manifestPath);
    if (!manifest.is_open()) {
        std::cerr << "Failed to create manifest file" << std::endl;
        return false;
    }

    manifest << "{\n";
    manifest << "  \"name\": \"MyAssetPack\",\n";
    manifest << "  \"version\": \"1.0.0\",\n";
    manifest << "  \"author\": \"Your Name\",\n";
    manifest << "  \"description\": \"Description of your asset pack\",\n";
    manifest << "  \"assets\": [\n";
    manifest << "    {\n";
    manifest << "      \"name\": \"ExampleTree\",\n";
    manifest << "      \"description\": \"An example tree asset for forests\",\n";
    manifest << "      \"modelPath\": \"models/tree.obj\",\n";
    manifest << "      \"texturePath\": \"textures/tree.png\",\n";
    manifest << "      \"biomes\": [\"Forest\", \"Plains\"],\n";
    manifest << "      \"placementRule\": \"Clustered\",\n";
    manifest << "      \"spawnChance\": 0.15,\n";
    manifest << "      \"minDistance\": 8.0,\n";
    manifest << "      \"minGroupSize\": 2,\n";
    manifest << "      \"maxGroupSize\": 8,\n";
    manifest << "      \"size\": [3.0, 12.0, 3.0],\n";
    manifest << "      \"offset\": [0.0, 0.0, 0.0],\n";
    manifest << "      \"randomRotation\": true,\n";
    manifest << "      \"rotationStep\": 90.0,\n";
    manifest << "      \"placeOnSurface\": true,\n";
    manifest << "      \"embedInTerrain\": false,\n";
    manifest << "      \"embedDepth\": 0.0,\n";
    manifest << "      \"tags\": [\"tree\", \"vegetation\", \"natural\"],\n";
    manifest << "      \"customProperties\": {\n";
    manifest << "        \"flammable\": \"true\",\n";
    manifest << "        \"harvestable\": \"true\"\n";
    manifest << "      }\n";
    manifest << "    },\n";
    manifest << "    {\n";
    manifest << "      \"name\": \"ExampleRock\",\n";
    manifest << "      \"description\": \"A large boulder for mountains\",\n";
    manifest << "      \"modelPath\": \"models/rock.obj\",\n";
    manifest << "      \"texturePath\": \"textures/rock.png\",\n";
    manifest << "      \"biomes\": [\"Mountains\", \"Plains\"],\n";
    manifest << "      \"placementRule\": \"Random\",\n";
    manifest << "      \"spawnChance\": 0.08,\n";
    manifest << "      \"minDistance\": 15.0,\n";
    manifest << "      \"size\": [2.5, 2.0, 2.5],\n";
    manifest << "      \"randomRotation\": true,\n";
    manifest << "      \"placeOnSurface\": true,\n";
    manifest << "      \"embedInTerrain\": true,\n";
    manifest << "      \"embedDepth\": 0.3,\n";
    manifest << "      \"tags\": [\"rock\", \"obstacle\", \"natural\"]\n";
    manifest << "    }\n";
    manifest << "  ]\n";
    manifest << "}\n";
    manifest.close();

    std::cout << "  Created manifest.json template" << std::endl;
    std::cout << "\n=== Asset Pack Template Created Successfully ===" << std::endl;
    std::cout << "Location: " << outputPath << std::endl;
    std::cout << "\nNext steps:" << std::endl;
    std::cout << "1. Read the README.md for detailed instructions" << std::endl;
    std::cout << "2. Edit manifest.json with your asset pack details" << std::endl;
    std::cout << "3. Add your model files to the models/ directory" << std::endl;
    std::cout << "4. Add your texture files to the textures/ directory" << std::endl;
    std::cout << "5. Place the pack folder in your assets directory" << std::endl;
    
    return true;
}

bool ModularAssetSystem::validateAssetPack(const std::string& packPath) const
{
    // Check if directory exists
    if (!std::filesystem::exists(packPath)) {
        std::cerr << "Pack directory does not exist: " << packPath << std::endl;
        return false;
    }

    // Check for manifest
    std::string manifestPath = packPath + "/manifest.json";
    if (!std::filesystem::exists(manifestPath)) {
        std::cerr << "Manifest file missing: " << manifestPath << std::endl;
        return false;
    }

    // Validate manifest structure and referenced files
#ifdef FRESH_JSON_AVAILABLE
    try {
        std::ifstream file(manifestPath);
        json manifest = json::parse(file);
        
        // Validate required fields
        if (!manifest.contains("name")) {
            std::cerr << "Manifest validation failed: missing 'name' field" << std::endl;
            return false;
        }
        if (!manifest.contains("version")) {
            std::cerr << "Manifest validation failed: missing 'version' field" << std::endl;
            return false;
        }
        if (!manifest.contains("assets")) {
            std::cerr << "Manifest validation failed: missing 'assets' field" << std::endl;
            return false;
        }
        
        // Validate asset references
        if (manifest["assets"].is_array()) {
            for (const auto& asset : manifest["assets"]) {
                if (asset.contains("path")) {
                    std::string assetPath = packPath + "/" + asset["path"].get<std::string>();
                    if (!std::filesystem::exists(assetPath)) {
                        std::cerr << "Manifest validation warning: referenced file not found: " 
                                  << assetPath << std::endl;
                    }
                }
            }
        }
        
        std::cout << "Manifest validation passed for: " << manifest["name"] << std::endl;
    } catch (const json::exception& e) {
        std::cerr << "Manifest validation failed: JSON parse error: " << e.what() << std::endl;
        return false;
    }
#else
    std::cout << "JSON library not available, skipping detailed manifest validation" << std::endl;
#endif

    return true;
}

void ModularAssetSystem::printStats() const
{
    std::cout << "\n=== Asset System Statistics ===" << std::endl;
    std::cout << "Loaded asset packs: " << loadedPacks.size() << std::endl;

    int totalAssets = 0;
    for (const auto& pack : loadedPacks) {
        totalAssets += static_cast<int>(pack->getAssets().size());
        std::cout << "  " << pack->getName() << " v" << pack->getVersion() << ": "
                  << pack->getAssets().size() << " assets" << std::endl;
    }

    std::cout << "Total assets: " << totalAssets << std::endl;
    std::cout << "Registered placement functions: " << placementFunctions.size() << std::endl;
}

void ModularAssetSystem::registerPlacementFunction(PlacementRule rule, PlacementFunction func)
{
    placementFunctions[rule] = func;
    std::cout << "Registered placement function for rule: " << static_cast<int>(rule) << std::endl;
}

// Placement implementations
std::vector<AssetInstance> ModularAssetSystem::placeRandomly(const AssetMetadata& asset,
                                                             VoxelWorld* world, int chunkX,
                                                             int chunkZ, uint32_t seed)
{
    std::vector<AssetInstance> instances;
    std::mt19937 rng(seed ^ (chunkX * 73856093) ^ (chunkZ * 19349663));
    std::uniform_real_distribution<float> posDist(0.0f, 16.0f);

    // Use the global CHUNK_SIZE constant
    glm::vec3 chunkWorldPos(chunkX * CHUNK_SIZE, 0, chunkZ * CHUNK_SIZE);

    // Try to place 1-3 instances
    std::uniform_int_distribution<int> countDist(1, 3);
    int attempts = countDist(rng);

    for (int i = 0; i < attempts; ++i) {
        glm::vec3 position = chunkWorldPos + glm::vec3(posDist(rng), 0, posDist(rng));

        // Get surface height
        // position.y = world->getSurfaceHeight(position.x, position.z);
        position.y = 64.0f; // Placeholder

        BiomeType biome = determineBiome(world, position);

        if (canPlaceAsset(asset, position, world, biome)) {
            AssetInstance instance;
            instance.assetName = asset.name;
            instance.position = position;
            instance.rotation =
                asset.randomRotation ? glm::vec3(0, posDist(rng) * 360.0f, 0) : glm::vec3(0);
            instance.biome = biome;
            instances.push_back(instance);
        }
    }

    return instances;
}

std::vector<AssetInstance> ModularAssetSystem::placeClustered(const AssetMetadata& asset,
                                                              VoxelWorld* world, int chunkX,
                                                              int chunkZ, uint32_t seed)
{
    std::vector<AssetInstance> instances;
    std::mt19937 rng(seed ^ (chunkX * 73856093) ^ (chunkZ * 19349663));
    std::uniform_real_distribution<float> posDist(0.0f, 16.0f);
    std::uniform_int_distribution<int> sizeDist(asset.minGroupSize, asset.maxGroupSize);

    // Use the global CHUNK_SIZE constant
    glm::vec3 chunkWorldPos(chunkX * CHUNK_SIZE, 0, chunkZ * CHUNK_SIZE);

    // Place cluster center
    glm::vec3 clusterCenter = chunkWorldPos + glm::vec3(posDist(rng), 0, posDist(rng));
    clusterCenter.y = 64.0f; // world->getSurfaceHeight(clusterCenter.x, clusterCenter.z);

    int clusterSize = sizeDist(rng);
    float clusterRadius = asset.minDistance * 0.5f;

    for (int i = 0; i < clusterSize; ++i) {
        // Offset from cluster center
        std::uniform_real_distribution<float> offsetDist(-clusterRadius, clusterRadius);
        glm::vec3 position = clusterCenter;
        position.x += offsetDist(rng);
        position.z += offsetDist(rng);
        position.y = 64.0f; // world->getSurfaceHeight(position.x, position.z);

        BiomeType biome = determineBiome(world, position);

        if (canPlaceAsset(asset, position, world, biome)) {
            AssetInstance instance;
            instance.assetName = asset.name;
            instance.position = position;
            instance.rotation =
                asset.randomRotation ? glm::vec3(0, posDist(rng) * 360.0f, 0) : glm::vec3(0);
            instance.biome = biome;
            instances.push_back(instance);
        }
    }

    return instances;
}

std::vector<AssetInstance> ModularAssetSystem::placeGrid(const AssetMetadata& asset,
                                                         VoxelWorld* world, int chunkX, int chunkZ,
                                                         uint32_t seed)
{
    (void)seed;  // Unused - could be used for variations in the future

    std::vector<AssetInstance> instances;
    // Use the global CHUNK_SIZE constant
    glm::vec3 chunkWorldPos(chunkX * CHUNK_SIZE, 0, chunkZ * CHUNK_SIZE);

    float spacing = asset.minDistance;

    for (float x = 0; x < CHUNK_SIZE; x += spacing) {
        for (float z = 0; z < CHUNK_SIZE; z += spacing) {
            glm::vec3 position = chunkWorldPos + glm::vec3(x, 0, z);
            position.y = 64.0f; // world->getSurfaceHeight(position.x, position.z);

            BiomeType biome = determineBiome(world, position);

            if (canPlaceAsset(asset, position, world, biome)) {
                AssetInstance instance;
                instance.assetName = asset.name;
                instance.position = position;
                instance.rotation = glm::vec3(0);
                instance.biome = biome;
                instances.push_back(instance);
            }
        }
    }

    return instances;
}

bool ModularAssetSystem::canPlaceAsset(const AssetMetadata& asset, const glm::vec3& position,
                                       VoxelWorld* world, BiomeType biome)
{
    (void)world;    // Unused - placeholder for future terrain queries
    // Check biome compatibility
    bool biomeAllowed = std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(), biome) !=
                        asset.allowedBiomes.end();
    bool anyBiome = std::find(asset.allowedBiomes.begin(), asset.allowedBiomes.end(),
                              BiomeType::Any) != asset.allowedBiomes.end();

    if (!biomeAllowed && !anyBiome) {
        return false;
    }

    // Check minimum distance to other assets of same type
    glm::vec2 pos2D(position.x, position.z);
    auto it = placedAssetPositions.find(asset.name);
    if (it != placedAssetPositions.end()) {
        for (const auto& placedPos : it->second) {
            float dist = glm::distance(pos2D, placedPos);
            if (dist < asset.minDistance) {
                return false;
            }
        }
    }

    // TODO: Additional checks (terrain slope, water proximity, etc.)

    return true;
}

BiomeType ModularAssetSystem::determineBiome(VoxelWorld* world, const glm::vec3& position)
{
    (void)world;
    (void)position; // Unused - placeholder for future implementation
    // TODO: Query world for actual biome
    // For now, return a placeholder
    return BiomeType::Plains;
}

} // namespace fresh
