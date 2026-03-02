#include <gtest/gtest.h>

#include "editor/ClientLauncher.h"
#include "editor/GamePackager.h"

using namespace fresh;

class ClientLauncherTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ClientLauncherTest, GetExecutableName_Client)
{
    std::string name = ClientLauncher::getExecutableName(LaunchTarget::Client);
    EXPECT_EQ(name, "FreshClient");
}

TEST_F(ClientLauncherTest, GetExecutableName_Server)
{
    std::string name = ClientLauncher::getExecutableName(LaunchTarget::Server);
    EXPECT_EQ(name, "FreshServer");
}

TEST_F(ClientLauncherTest, GetExecutableName_Runtime)
{
    std::string name = ClientLauncher::getExecutableName(LaunchTarget::Runtime);
    EXPECT_EQ(name, "FreshRuntime");
}

TEST_F(ClientLauncherTest, BuildCommandLine_WithProjectAndWorld)
{
    LaunchConfig config;
    config.projectPath = "/my/project";
    config.worldPath = "/my/project/saves/world.save";
    config.windowed = true;
    config.windowWidth = 1920;
    config.windowHeight = 1080;

    std::string cmdLine = ClientLauncher::buildCommandLine(config);

    EXPECT_NE(cmdLine.find("--project"), std::string::npos);
    EXPECT_NE(cmdLine.find("/my/project"), std::string::npos);
    EXPECT_NE(cmdLine.find("--world"), std::string::npos);
    EXPECT_NE(cmdLine.find("--windowed"), std::string::npos);
    EXPECT_NE(cmdLine.find("--width 1920"), std::string::npos);
    EXPECT_NE(cmdLine.find("--height 1080"), std::string::npos);
}

TEST_F(ClientLauncherTest, BuildCommandLine_EmptyPaths_OmitsArgs)
{
    LaunchConfig config;
    config.projectPath = "";
    config.worldPath = "";
    config.windowed = false;
    config.windowWidth = 800;
    config.windowHeight = 600;

    std::string cmdLine = ClientLauncher::buildCommandLine(config);

    EXPECT_EQ(cmdLine.find("--project"), std::string::npos);
    EXPECT_EQ(cmdLine.find("--world"), std::string::npos);
    EXPECT_EQ(cmdLine.find("--windowed"), std::string::npos);
    EXPECT_NE(cmdLine.find("--width 800"), std::string::npos);
    EXPECT_NE(cmdLine.find("--height 600"), std::string::npos);
}

TEST_F(ClientLauncherTest, SetExecutableDir_StoresValue)
{
    ClientLauncher launcher;
    launcher.setExecutableDir("/path/to/build");

    EXPECT_EQ(launcher.getExecutableDir(), "/path/to/build");
}

TEST_F(ClientLauncherTest, Launch_NonexistentExecutable_Fails)
{
    ClientLauncher launcher;
    launcher.setExecutableDir("/nonexistent/path");

    LaunchConfig config;
    config.target = LaunchTarget::Client;
    config.executableDir = "/nonexistent/path";

    LaunchResult result = launcher.launch(config);

    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.empty());
}

TEST_F(ClientLauncherTest, LaunchConfig_DefaultValues)
{
    LaunchConfig config;

    EXPECT_EQ(config.target, LaunchTarget::Client);
    EXPECT_TRUE(config.projectPath.empty());
    EXPECT_TRUE(config.worldPath.empty());
    EXPECT_EQ(config.windowWidth, 1280);
    EXPECT_EQ(config.windowHeight, 720);
    EXPECT_TRUE(config.windowed);
}

TEST_F(ClientLauncherTest, PackageConfig_DefaultValues)
{
    PackageConfig config;

    EXPECT_TRUE(config.projectName.empty());
    EXPECT_TRUE(config.includeAssets);
    EXPECT_TRUE(config.includeShaders);
    EXPECT_TRUE(config.includeSounds);
    EXPECT_TRUE(config.includeTextures);
    EXPECT_TRUE(config.includeConfig);
    EXPECT_TRUE(config.includeScripts);
}

TEST_F(ClientLauncherTest, LaunchResult_DefaultValues)
{
    LaunchResult result;

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.errorMessage.empty());
    EXPECT_TRUE(result.commandLine.empty());
    EXPECT_EQ(result.processId, 0);
}
