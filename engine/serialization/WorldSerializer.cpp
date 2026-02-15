#include "serialization/WorldSerializer.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "voxel/Chunk.h"
#include "voxel/VoxelWorld.h"

namespace fs = std::filesystem;

namespace fresh
{

WorldSerializer::WorldSerializer() {}

WorldSerializer::~WorldSerializer() {}

bool WorldSerializer::saveWorld(const VoxelWorld* world, const std::string& filename)
{
    if (!world) {
        std::cerr << "Cannot save null world" << std::endl;
        return false;
    }

    try {
        // Ensure saves directory exists
        fs::path savePath(filename);
        fs::create_directories(savePath.parent_path());

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        // Write header
        const char* magic = "FVEW"; // Fresh Voxel Engine World
        file.write(magic, 4);

        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // Write number of chunks
        uint32_t chunkCount = static_cast<uint32_t>(world->getChunks().size());
        file.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

        std::cout << "Saving " << chunkCount << " chunks..." << std::endl;

        // Write each chunk
        int savedChunks = 0;
        for (const auto& [pos, chunk] : world->getChunks()) {
            // Write chunk position
            file.write(reinterpret_cast<const char*>(&pos.x), sizeof(pos.x));
            file.write(reinterpret_cast<const char*>(&pos.z), sizeof(pos.z));

            // Write voxel data (simplified - write all voxels)
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    for (int x = 0; x < CHUNK_SIZE; ++x) {
                        const Voxel& voxel = chunk->getVoxel(x, y, z);
                        file.write(reinterpret_cast<const char*>(&voxel.type), sizeof(voxel.type));
                        file.write(reinterpret_cast<const char*>(&voxel.light),
                                   sizeof(voxel.light));
                    }
                }
            }

            savedChunks++;
            if (savedChunks % 10 == 0) {
                std::cout << "  Saved " << savedChunks << "/" << chunkCount << " chunks..."
                          << std::endl;
            }
        }

        file.close();
        std::cout << "World saved successfully to " << filename << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving world: " << e.what() << std::endl;
        return false;
    }
}

bool WorldSerializer::loadWorld(VoxelWorld* world, const std::string& filename)
{
    if (!world) {
        std::cerr << "Cannot load into null world" << std::endl;
        return false;
    }

    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filename << std::endl;
            return false;
        }

        // Read and verify header
        char magic[4];
        file.read(magic, 4);
        if (std::string(magic, 4) != "FVEW") {
            std::cerr << "Invalid world file format" << std::endl;
            return false;
        }

        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));

        if (version != 1) {
            std::cerr << "Unsupported world file version: " << version << std::endl;
            return false;
        }

        // Read number of chunks
        uint32_t chunkCount;
        file.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));

        std::cout << "Loading " << chunkCount << " chunks..." << std::endl;

        // Read each chunk
        for (uint32_t i = 0; i < chunkCount; ++i) {
            // Read chunk position
            ChunkPos pos;
            file.read(reinterpret_cast<char*>(&pos.x), sizeof(pos.x));
            file.read(reinterpret_cast<char*>(&pos.z), sizeof(pos.z));

            // Create chunk
            Chunk* chunk = world->loadChunk(pos);
            if (!chunk) {
                std::cerr << "Failed to create chunk at " << pos.x << ", " << pos.z << std::endl;
                continue;
            }

            // Read voxel data
            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    for (int x = 0; x < CHUNK_SIZE; ++x) {
                        Voxel voxel;
                        file.read(reinterpret_cast<char*>(&voxel.type), sizeof(voxel.type));
                        file.read(reinterpret_cast<char*>(&voxel.light), sizeof(voxel.light));
                        chunk->setVoxel(x, y, z, voxel);
                    }
                }
            }

            // Regenerate mesh
            chunk->generateMesh();

            if ((i + 1) % 10 == 0) {
                std::cout << "  Loaded " << (i + 1) << "/" << chunkCount << " chunks..."
                          << std::endl;
            }
        }

        file.close();
        std::cout << "World loaded successfully from " << filename << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading world: " << e.what() << std::endl;
        return false;
    }
}

} // namespace fresh
