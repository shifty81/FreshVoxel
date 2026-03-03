#include <gtest/gtest.h>
#include "core/GameStateManager.h"

class GameStateManagerTest : public ::testing::Test {
protected:
    fresh::GameStateManager gsm;
};

TEST_F(GameStateManagerTest, DefaultPhase_IsMainMenu) {
    EXPECT_EQ(gsm.phase(), fresh::GamePhase::MainMenu);
}

TEST_F(GameStateManagerTest, SetPhase_ChangesPhase) {
    gsm.setPhase(fresh::GamePhase::Playing);
    EXPECT_EQ(gsm.phase(), fresh::GamePhase::Playing);
}

TEST_F(GameStateManagerTest, PhaseCallback_FiresOnChange) {
    fresh::GamePhase oldPhase = fresh::GamePhase::MainMenu;
    fresh::GamePhase newPhase = fresh::GamePhase::MainMenu;
    int callCount = 0;

    gsm.onPhaseChange([&](fresh::GamePhase o, fresh::GamePhase n) {
        oldPhase = o;
        newPhase = n;
        callCount++;
    });

    gsm.setPhase(fresh::GamePhase::Loading);
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(oldPhase, fresh::GamePhase::MainMenu);
    EXPECT_EQ(newPhase, fresh::GamePhase::Loading);
}

TEST_F(GameStateManagerTest, SetSamePhase_DoesNotFireCallback) {
    int callCount = 0;
    gsm.onPhaseChange([&](fresh::GamePhase, fresh::GamePhase) { callCount++; });

    gsm.setPhase(fresh::GamePhase::MainMenu);
    EXPECT_EQ(callCount, 0);
}

TEST_F(GameStateManagerTest, Reset_RestoresDefaults) {
    gsm.setPhase(fresh::GamePhase::Playing);
    gsm.setPlayerEntity(42);
    gsm.setCurrentWorld(7);

    gsm.reset();
    EXPECT_EQ(gsm.phase(), fresh::GamePhase::MainMenu);
    EXPECT_EQ(gsm.playerEntity(), 0u);
    EXPECT_EQ(gsm.currentWorld(), 0u);
}

TEST_F(GameStateManagerTest, GamePhaseName_ReturnsCorrectStrings) {
    EXPECT_STREQ(fresh::gamePhaseName(fresh::GamePhase::MainMenu), "MainMenu");
    EXPECT_STREQ(fresh::gamePhaseName(fresh::GamePhase::Loading), "Loading");
    EXPECT_STREQ(fresh::gamePhaseName(fresh::GamePhase::Playing), "Playing");
    EXPECT_STREQ(fresh::gamePhaseName(fresh::GamePhase::Editing), "Editing");
    EXPECT_STREQ(fresh::gamePhaseName(fresh::GamePhase::Paused), "Paused");
}
