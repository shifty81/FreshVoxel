#include "core/GameStateManager.h"

namespace fresh {

const char* gamePhaseName(GamePhase phase) {
    switch (phase) {
        case GamePhase::MainMenu: return "MainMenu";
        case GamePhase::Loading:  return "Loading";
        case GamePhase::Playing:  return "Playing";
        case GamePhase::Editing:  return "Editing";
        case GamePhase::Paused:   return "Paused";
    }
    return "Unknown";
}

void GameStateManager::setPhase(GamePhase phase) {
    if (phase == m_phase) return;
    GamePhase old = m_phase;
    m_phase = phase;
    // Defensive copy: callbacks may modify the subscription list during iteration
    auto callbacks = m_phaseCallbacks;
    for (auto& cb : callbacks) {
        if (cb) cb(old, m_phase);
    }
}

void GameStateManager::onPhaseChange(PhaseCallback cb) {
    if (cb) m_phaseCallbacks.push_back(std::move(cb));
}

void GameStateManager::reset() {
    m_phase = GamePhase::MainMenu;
    m_playerEntity = 0;
    m_currentWorld = 0;
}

} // namespace fresh
