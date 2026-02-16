#include "CommandHistory.h"
#include "core/Logger.h"

namespace fresh {

void CommandHistory::execute(std::unique_ptr<Command> cmd) {
    cmd->execute();
    Logger::getInstance().info("Command executed: " + cmd->name(), "CommandHistory");
    m_undoStack.push_back(std::move(cmd));
    m_redoStack.clear();
}

bool CommandHistory::canUndo() const {
    return !m_undoStack.empty();
}

bool CommandHistory::canRedo() const {
    return !m_redoStack.empty();
}

void CommandHistory::undo() {
    if (m_undoStack.empty()) return;

    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    cmd->undo();
    Logger::getInstance().info("Command undone: " + cmd->name(), "CommandHistory");
    m_redoStack.push_back(std::move(cmd));
}

void CommandHistory::redo() {
    if (m_redoStack.empty()) return;

    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    cmd->execute();
    Logger::getInstance().info("Command redone: " + cmd->name(), "CommandHistory");
    m_undoStack.push_back(std::move(cmd));
}

void CommandHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

size_t CommandHistory::undoCount() const {
    return m_undoStack.size();
}

size_t CommandHistory::redoCount() const {
    return m_redoStack.size();
}

const Command* CommandHistory::lastExecuted() const {
    if (m_undoStack.empty()) return nullptr;
    return m_undoStack.back().get();
}

} // namespace fresh
