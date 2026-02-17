#include <gtest/gtest.h>
#include "devtools/TimeCommands.h"
#include "devtools/DebugConsole.h"
#include "gameplay/TimeManager.h"

using namespace fresh;
using namespace fresh::devtools;

class TimeCommandsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        console = std::make_unique<DebugConsole>();
        timeManager = std::make_unique<TimeManager>();
        console->initialize();
        TimeCommands::registerCommands(console.get(), timeManager.get());
    }

    void TearDown() override
    {
        TimeCommands::unregisterCommands(console.get());
        console->shutdown();
        timeManager.reset();
        console.reset();
    }

    std::unique_ptr<DebugConsole> console;
    std::unique_ptr<TimeManager> timeManager;
};

// Test command registration
TEST_F(TimeCommandsTest, CommandsAreRegistered)
{
    // Execute "time" command - should not produce an error
    console->executeCommand("time");
    
    // Check that output log has the time info (not an error)
    const auto& log = console->getLog();
    EXPECT_FALSE(log.empty());
    
    // Should contain time info, not "Unknown command"
    bool hasTimeInfo = false;
    for (const auto& line : log) {
        if (line.find("Current time") != std::string::npos) {
            hasTimeInfo = true;
            break;
        }
    }
    EXPECT_TRUE(hasTimeInfo);
}

// Test time set commands
TEST_F(TimeCommandsTest, SetTimeDawn)
{
    console->executeCommand("time set dawn");
    EXPECT_EQ(timeManager->getTime(), 6000);
}

TEST_F(TimeCommandsTest, SetTimeNoon)
{
    console->executeCommand("time set noon");
    EXPECT_EQ(timeManager->getTime(), 12000);
}

TEST_F(TimeCommandsTest, SetTimeDusk)
{
    console->executeCommand("time set dusk");
    EXPECT_EQ(timeManager->getTime(), 18000);
}

TEST_F(TimeCommandsTest, SetTimeMidnight)
{
    console->executeCommand("time set midnight");
    EXPECT_EQ(timeManager->getTime(), 0);
}

TEST_F(TimeCommandsTest, SetTimeTicks)
{
    console->executeCommand("time set 15000");
    EXPECT_EQ(timeManager->getTime(), 15000);
}

TEST_F(TimeCommandsTest, SetTimeTicksOutOfRange)
{
    int originalTime = timeManager->getTime();
    console->executeCommand("time set 30000");
    // Should not change time
    EXPECT_EQ(timeManager->getTime(), originalTime);
    
    // Should have error in log
    const auto& log = console->getLog();
    bool hasError = false;
    for (const auto& line : log) {
        if (line.find("ERROR") != std::string::npos) {
            hasError = true;
            break;
        }
    }
    EXPECT_TRUE(hasError);
}

TEST_F(TimeCommandsTest, SetTimeInvalidValue)
{
    int originalTime = timeManager->getTime();
    console->executeCommand("time set invalid");
    // Should not change time
    EXPECT_EQ(timeManager->getTime(), originalTime);
}

// Test time rate commands
TEST_F(TimeCommandsTest, SetTimeRate)
{
    console->executeCommand("time rate 2.5");
    EXPECT_FLOAT_EQ(timeManager->getTimeRate(), 2.5f);
}

TEST_F(TimeCommandsTest, SetTimeRateZero)
{
    console->executeCommand("time rate 0");
    EXPECT_FLOAT_EQ(timeManager->getTimeRate(), 0.0f);
}

TEST_F(TimeCommandsTest, SetTimeRateNegative)
{
    float originalRate = timeManager->getTimeRate();
    console->executeCommand("time rate -1");
    // Should show error or clamp to 0
    EXPECT_GE(timeManager->getTimeRate(), 0.0f);
    
    // Check for error in log
    const auto& log = console->getLog();
    bool hasError = false;
    for (const auto& line : log) {
        if (line.find("ERROR") != std::string::npos) {
            hasError = true;
            break;
        }
    }
    EXPECT_TRUE(hasError);
}

// Test pause/resume commands
TEST_F(TimeCommandsTest, PauseTime)
{
    EXPECT_FALSE(timeManager->isPaused());
    console->executeCommand("time pause");
    EXPECT_TRUE(timeManager->isPaused());
}

TEST_F(TimeCommandsTest, ResumeTime)
{
    timeManager->pause();
    EXPECT_TRUE(timeManager->isPaused());
    console->executeCommand("time resume");
    EXPECT_FALSE(timeManager->isPaused());
}

TEST_F(TimeCommandsTest, ToggleTime)
{
    EXPECT_FALSE(timeManager->isPaused());
    console->executeCommand("time toggle");
    EXPECT_TRUE(timeManager->isPaused());
    console->executeCommand("time toggle");
    EXPECT_FALSE(timeManager->isPaused());
}

// Test command unregistration
TEST_F(TimeCommandsTest, UnregisterCommands)
{
    TimeCommands::unregisterCommands(console.get());
    
    // Clear the log using the default 'clear' command (registered by DebugConsole::initialize)
    console->executeCommand("clear");
    
    // Execute "time" command - should produce "Unknown command" error since it was unregistered
    console->executeCommand("time");
    
    const auto& log = console->getLog();
    bool hasUnknownError = false;
    for (const auto& line : log) {
        if (line.find("Unknown command") != std::string::npos) {
            hasUnknownError = true;
            break;
        }
    }
    EXPECT_TRUE(hasUnknownError);
    
    // Re-register for TearDown
    TimeCommands::registerCommands(console.get(), timeManager.get());
}

// Test case insensitivity
TEST_F(TimeCommandsTest, CaseInsensitiveCommands)
{
    console->executeCommand("time set NOON");
    EXPECT_EQ(timeManager->getTime(), 12000);
    
    console->executeCommand("time set Dusk");
    EXPECT_EQ(timeManager->getTime(), 18000);
}

// Test null safety
TEST_F(TimeCommandsTest, NullConsoleSafety)
{
    // Should not crash
    TimeCommands::registerCommands(nullptr, timeManager.get());
    TimeCommands::unregisterCommands(nullptr);
}

TEST_F(TimeCommandsTest, NullTimeManagerSafety)
{
    // Should not crash
    TimeCommands::registerCommands(console.get(), nullptr);
}
