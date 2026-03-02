#include <gtest/gtest.h>

#include "editor/GamePackager.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

using namespace fresh;

class GamePackagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary project directory structure for testing
        m_testDir = "test_packager_project";
        fs::create_directories(m_testDir + "/saves");
        fs::create_directories(m_testDir + "/shaders");
        fs::create_directories(m_testDir + "/sounds");
        fs::create_directories(m_testDir + "/textures");
        fs::create_directories(m_testDir + "/config");
        fs::create_directories(m_testDir + "/scripts");
        fs::create_directories(m_testDir + "/asset_packs");

        // Create test files
        createTestFile(m_testDir + "/saves/test.world", "world data");
        createTestFile(m_testDir + "/shaders/voxel.hlsl", "shader code");
        createTestFile(m_testDir + "/sounds/click.wav", "audio data");
        createTestFile(m_testDir + "/textures/stone.png", "texture data");
        createTestFile(m_testDir + "/config/game.json", "{\"key\": \"value\"}");
        createTestFile(m_testDir + "/scripts/init.lua", "print('hello')");
        createTestFile(m_testDir + "/asset_packs/manifest.json", "{\"name\": \"default\"}");

        // Create output directory
        m_outputDir = "test_packager_output";
        fs::create_directories(m_outputDir);
    }

    void TearDown() override
    {
        // Clean up test directories
        fs::remove_all(m_testDir);
        fs::remove_all(m_outputDir);
    }

    void createTestFile(const std::string& path, const std::string& content)
    {
        std::ofstream file(path);
        file << content;
        file.close();
    }

    std::string m_testDir;
    std::string m_outputDir;
};

TEST_F(GamePackagerTest, BuildPackage_EmptyProjectName_Fails)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "";
    config.outputPath = m_outputDir;

    PackageResult result = packager.buildPackage(config);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(GamePackagerTest, BuildPackage_EmptyOutputPath_Fails)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.outputPath = "";

    PackageResult result = packager.buildPackage(config);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(GamePackagerTest, BuildPackage_ValidConfig_Succeeds)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.worldSavePath = m_testDir + "/saves/test.world";

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_FALSE(result.filesIncluded.empty());
    EXPECT_GT(result.totalSizeBytes, 0u);

    // Verify package directory was created
    EXPECT_TRUE(fs::exists(m_outputDir + "/TestGame"));
}

TEST_F(GamePackagerTest, BuildPackage_IncludesWorldSave)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.worldSavePath = m_testDir + "/saves/test.world";

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(fs::exists(m_outputDir + "/TestGame/saves/test.world"));
}

TEST_F(GamePackagerTest, BuildPackage_IncludesShaders)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.includeShaders = true;

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(fs::exists(m_outputDir + "/TestGame/shaders/voxel.hlsl"));
}

TEST_F(GamePackagerTest, BuildPackage_IncludesScripts)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.includeScripts = true;

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(fs::exists(m_outputDir + "/TestGame/scripts/init.lua"));
}

TEST_F(GamePackagerTest, BuildPackage_CreatesManifest)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.worldSavePath = m_testDir + "/saves/test.world";

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    EXPECT_TRUE(fs::exists(m_outputDir + "/TestGame/package.json"));

    // Verify manifest content
    std::ifstream manifestFile(m_outputDir + "/TestGame/package.json");
    std::string manifestContent((std::istreambuf_iterator<char>(manifestFile)),
                                 std::istreambuf_iterator<char>());
    EXPECT_NE(manifestContent.find("TestGame"), std::string::npos);
    EXPECT_NE(manifestContent.find("FreshVoxel"), std::string::npos);
    EXPECT_NE(manifestContent.find("FreshClient"), std::string::npos);
}

TEST_F(GamePackagerTest, BuildPackage_ExcludesDisabledCategories)
{
    GamePackager packager;
    PackageConfig config;
    config.projectName = "TestGame";
    config.projectPath = m_testDir;
    config.outputPath = m_outputDir;
    config.includeShaders = false;
    config.includeSounds = false;
    config.includeTextures = false;
    config.includeScripts = false;

    PackageResult result = packager.buildPackage(config);

    EXPECT_TRUE(result.success);
    // These should NOT exist since we disabled them
    EXPECT_FALSE(fs::exists(m_outputDir + "/TestGame/shaders/voxel.hlsl"));
    EXPECT_FALSE(fs::exists(m_outputDir + "/TestGame/sounds/click.wav"));
    EXPECT_FALSE(fs::exists(m_outputDir + "/TestGame/textures/stone.png"));
    EXPECT_FALSE(fs::exists(m_outputDir + "/TestGame/scripts/init.lua"));
}

TEST_F(GamePackagerTest, CollectFiles_FindsMatchingExtensions)
{
    std::vector<std::string> files;
    GamePackager::collectFiles(m_testDir + "/shaders", {".hlsl"}, files);

    EXPECT_EQ(files.size(), 1u);
}

TEST_F(GamePackagerTest, CollectFiles_EmptyDirectory_ReturnsEmpty)
{
    fs::create_directories(m_testDir + "/empty_dir");
    std::vector<std::string> files;
    GamePackager::collectFiles(m_testDir + "/empty_dir", {".txt"}, files);

    EXPECT_TRUE(files.empty());
}

TEST_F(GamePackagerTest, CollectFiles_NonexistentDirectory_ReturnsEmpty)
{
    std::vector<std::string> files;
    GamePackager::collectFiles("/nonexistent/path", {".txt"}, files);

    EXPECT_TRUE(files.empty());
}

TEST_F(GamePackagerTest, CreateManifest_ContainsRequiredFields)
{
    PackageConfig config;
    config.projectName = "MyGame";
    config.worldSavePath = "saves/world.save";

    std::vector<std::string> files = {"file1.txt", "file2.txt"};

    std::string manifest = GamePackager::createManifest(config, files);

    EXPECT_NE(manifest.find("MyGame"), std::string::npos);
    EXPECT_NE(manifest.find("FreshVoxel"), std::string::npos);
    EXPECT_NE(manifest.find("file_count"), std::string::npos);
    EXPECT_NE(manifest.find("packaged_at"), std::string::npos);
}
