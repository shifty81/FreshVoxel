#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace fresh {

enum class GamePhase : uint8_t {
    MainMenu,
    Loading,
    Playing,
    Editing,
    Paused
};

const char* gamePhaseName(GamePhase phase);

class GameStateManager {
public:
    GamePhase phase() const { return m_phase; }
    void setPhase(GamePhase phase);

    using PhaseCallback = std::function<void(GamePhase, GamePhase)>;
    void onPhaseChange(PhaseCallback cb);

    uint32_t playerEntity() const { return m_playerEntity; }
    void setPlayerEntity(uint32_t id) { m_playerEntity = id; }

    uint32_t currentWorld() const { return m_currentWorld; }
    void setCurrentWorld(uint32_t worldId) { m_currentWorld = worldId; }

    void reset();

private:
    GamePhase m_phase = GamePhase::MainMenu;
    uint32_t m_playerEntity = 0;
    uint32_t m_currentWorld = 0;
    std::vector<PhaseCallback> m_phaseCallbacks;
};

} // namespace fresh
