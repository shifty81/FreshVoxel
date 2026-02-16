#pragma once
#include <string>
#include <vector>
#include <memory>

namespace fresh {

class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;

    virtual std::string name() const = 0;
};

class CommandHistory {
public:
    void execute(std::unique_ptr<Command> cmd);

    bool canUndo() const;
    bool canRedo() const;

    void undo();
    void redo();

    void clear();

    size_t undoCount() const;
    size_t redoCount() const;

    const Command* lastExecuted() const;

private:
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};

} // namespace fresh
