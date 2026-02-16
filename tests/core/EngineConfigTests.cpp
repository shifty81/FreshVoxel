/**
 * @file EngineConfigTests.cpp
 * @brief Tests for EngineConfig, EngineMode, and LastSessionConfig
 *
 * These tests verify that the EngineConfig struct correctly configures
 * engine modes for Editor, Client, Server, and Runtime executables,
 * ensuring proper separation of concerns between the four targets.
 * Also verifies that LastSessionConfig correctly persists the last
 * saved world path so client/runtime can auto-load editor changes.
 */

#include <gtest/gtest.h>
#include "core/EngineConfig.h"
#include "core/LastSessionConfig.h"
#include <filesystem>
#include <fstream>

namespace fresh {

// ---------------------------------------------------------------------------
// EngineMode name tests
// ---------------------------------------------------------------------------

TEST(EngineModeTest, GetEngineModeName_ReturnsCorrectStrings) {
    EXPECT_STREQ(getEngineModeName(EngineMode::Editor), "Editor");
    EXPECT_STREQ(getEngineModeName(EngineMode::Client), "Client");
    EXPECT_STREQ(getEngineModeName(EngineMode::Server), "Server");
    EXPECT_STREQ(getEngineModeName(EngineMode::Runtime), "Runtime");
}

// ---------------------------------------------------------------------------
// EngineConfig default creation tests
// ---------------------------------------------------------------------------

TEST(EngineConfigTest, EditorMode_HasCorrectDefaults) {
    auto config = EngineConfig::createDefault(EngineMode::Editor);

    EXPECT_EQ(config.mode, EngineMode::Editor);
    EXPECT_TRUE(config.enableRendering);
    EXPECT_TRUE(config.enableEditor);
    EXPECT_FALSE(config.enableNetworking);
    EXPECT_FALSE(config.headless);
    EXPECT_TRUE(config.hasWindow());
    EXPECT_TRUE(config.isEditor());
    EXPECT_FALSE(config.isClient());
    EXPECT_FALSE(config.isServer());
    EXPECT_FALSE(config.isRuntime());
}

TEST(EngineConfigTest, ClientMode_HasCorrectDefaults) {
    auto config = EngineConfig::createDefault(EngineMode::Client);

    EXPECT_EQ(config.mode, EngineMode::Client);
    EXPECT_TRUE(config.enableRendering);
    EXPECT_FALSE(config.enableEditor);
    EXPECT_TRUE(config.enableNetworking);
    EXPECT_FALSE(config.headless);
    EXPECT_TRUE(config.hasWindow());
    EXPECT_FALSE(config.isEditor());
    EXPECT_TRUE(config.isClient());
    EXPECT_FALSE(config.isServer());
    EXPECT_FALSE(config.isRuntime());
}

TEST(EngineConfigTest, ServerMode_HasCorrectDefaults) {
    auto config = EngineConfig::createDefault(EngineMode::Server);

    EXPECT_EQ(config.mode, EngineMode::Server);
    EXPECT_FALSE(config.enableRendering);
    EXPECT_FALSE(config.enableEditor);
    EXPECT_TRUE(config.enableNetworking);
    EXPECT_TRUE(config.headless);
    EXPECT_FALSE(config.hasWindow());
    EXPECT_FALSE(config.isEditor());
    EXPECT_FALSE(config.isClient());
    EXPECT_TRUE(config.isServer());
    EXPECT_FALSE(config.isRuntime());
}

TEST(EngineConfigTest, RuntimeMode_HasCorrectDefaults) {
    auto config = EngineConfig::createDefault(EngineMode::Runtime);

    EXPECT_EQ(config.mode, EngineMode::Runtime);
    EXPECT_TRUE(config.enableRendering);
    EXPECT_FALSE(config.enableEditor);
    EXPECT_FALSE(config.enableNetworking);
    EXPECT_FALSE(config.headless);
    EXPECT_TRUE(config.hasWindow());
    EXPECT_FALSE(config.isEditor());
    EXPECT_FALSE(config.isClient());
    EXPECT_FALSE(config.isServer());
    EXPECT_TRUE(config.isRuntime());
}

// ---------------------------------------------------------------------------
// EngineConfig custom configuration tests
// ---------------------------------------------------------------------------

TEST(EngineConfigTest, CustomConfig_CanOverrideDefaults) {
    auto config = EngineConfig::createDefault(EngineMode::Client);

    // Override defaults
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.windowTitle = "My Custom Game";

    EXPECT_EQ(config.windowWidth, 1920);
    EXPECT_EQ(config.windowHeight, 1080);
    EXPECT_EQ(config.windowTitle, "My Custom Game");
    // Mode-specific defaults should still be correct
    EXPECT_TRUE(config.enableRendering);
    EXPECT_FALSE(config.enableEditor);
}

TEST(EngineConfigTest, HasWindow_FalseWhenHeadless) {
    EngineConfig config;
    config.enableRendering = true;
    config.headless = true;

    EXPECT_FALSE(config.hasWindow());
}

TEST(EngineConfigTest, HasWindow_FalseWhenRenderingDisabled) {
    EngineConfig config;
    config.enableRendering = false;
    config.headless = false;

    EXPECT_FALSE(config.hasWindow());
}

TEST(EngineConfigTest, HasWindow_TrueWhenRenderingAndNotHeadless) {
    EngineConfig config;
    config.enableRendering = true;
    config.headless = false;

    EXPECT_TRUE(config.hasWindow());
}

// ---------------------------------------------------------------------------
// EngineConfig default constructor test
// ---------------------------------------------------------------------------

TEST(EngineConfigTest, DefaultConstructor_CreatesEditorConfig) {
    EngineConfig config;

    EXPECT_EQ(config.mode, EngineMode::Editor);
    EXPECT_TRUE(config.enableRendering);
    EXPECT_TRUE(config.enableEditor);
    EXPECT_FALSE(config.enableNetworking);
    EXPECT_FALSE(config.headless);
    EXPECT_EQ(config.windowWidth, 1280);
    EXPECT_EQ(config.windowHeight, 720);
}

// ---------------------------------------------------------------------------
// Window title tests
// ---------------------------------------------------------------------------

TEST(EngineConfigTest, EditorMode_HasEditorWindowTitle) {
    auto config = EngineConfig::createDefault(EngineMode::Editor);
    EXPECT_NE(config.windowTitle.find("Editor"), std::string::npos);
}

TEST(EngineConfigTest, ServerMode_HasServerWindowTitle) {
    auto config = EngineConfig::createDefault(EngineMode::Server);
    EXPECT_NE(config.windowTitle.find("Server"), std::string::npos);
}

// ---------------------------------------------------------------------------
// autoLoadLastWorld tests
// ---------------------------------------------------------------------------

TEST(EngineConfigTest, EditorMode_DoesNotAutoLoadLastWorld) {
    auto config = EngineConfig::createDefault(EngineMode::Editor);
    EXPECT_FALSE(config.autoLoadLastWorld);
}

TEST(EngineConfigTest, ClientMode_AutoLoadsLastWorld) {
    auto config = EngineConfig::createDefault(EngineMode::Client);
    EXPECT_TRUE(config.autoLoadLastWorld);
}

TEST(EngineConfigTest, ServerMode_AutoLoadsLastWorld) {
    auto config = EngineConfig::createDefault(EngineMode::Server);
    EXPECT_TRUE(config.autoLoadLastWorld);
}

TEST(EngineConfigTest, RuntimeMode_AutoLoadsLastWorld) {
    auto config = EngineConfig::createDefault(EngineMode::Runtime);
    EXPECT_TRUE(config.autoLoadLastWorld);
}

// ---------------------------------------------------------------------------
// LastSessionConfig tests
// ---------------------------------------------------------------------------

class LastSessionConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Clean up any existing config file
        std::filesystem::remove("config/last_session.cfg");
    }

    void TearDown() override {
        // Clean up after test
        std::filesystem::remove("config/last_session.cfg");
    }
};

TEST_F(LastSessionConfigTest, SaveAndLoad_RoundTrips) {
    const std::string testPath = "saves/test_world.world";
    LastSessionConfig::saveLastWorldPath(testPath);

    std::string loaded = LastSessionConfig::loadLastWorldPath();
    EXPECT_EQ(loaded, testPath);
}

TEST_F(LastSessionConfigTest, Load_ReturnsEmpty_WhenNoFile) {
    std::string loaded = LastSessionConfig::loadLastWorldPath();
    EXPECT_TRUE(loaded.empty());
}

TEST_F(LastSessionConfigTest, Save_OverwritesPrevious) {
    LastSessionConfig::saveLastWorldPath("saves/old_world.world");
    LastSessionConfig::saveLastWorldPath("saves/new_world.world");

    std::string loaded = LastSessionConfig::loadLastWorldPath();
    EXPECT_EQ(loaded, "saves/new_world.world");
}

TEST_F(LastSessionConfigTest, Save_CreatesConfigDirectory) {
    // Remove config directory if it exists
    std::filesystem::remove_all("config");

    LastSessionConfig::saveLastWorldPath("saves/test.world");

    EXPECT_TRUE(std::filesystem::exists("config/last_session.cfg"));
}

} // namespace fresh
