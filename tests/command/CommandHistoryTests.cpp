#include <gtest/gtest.h>
#include "command/CommandHistory.h"
#include <string>

namespace {

// Test command that tracks execution
class IncrementCommand : public fresh::Command {
public:
    IncrementCommand(int& value) : m_value(value) {}

    void execute() override { m_value++; }
    void undo() override { m_value--; }
    std::string name() const override { return "Increment"; }

private:
    int& m_value;
};

class SetValueCommand : public fresh::Command {
public:
    SetValueCommand(int& target, int newVal)
        : m_target(target), m_newVal(newVal), m_oldVal(target) {}

    void execute() override { m_target = m_newVal; }
    void undo() override { m_target = m_oldVal; }
    std::string name() const override { return "SetValue"; }

private:
    int& m_target;
    int m_newVal;
    int m_oldVal;
};

} // namespace

class CommandHistoryTest : public ::testing::Test {
protected:
    fresh::CommandHistory history;
    int testValue = 0;
};

TEST_F(CommandHistoryTest, InitialState) {
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.undoCount(), 0u);
    EXPECT_EQ(history.redoCount(), 0u);
    EXPECT_EQ(history.lastExecuted(), nullptr);
}

TEST_F(CommandHistoryTest, ExecuteCommand) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    EXPECT_EQ(testValue, 1);
    EXPECT_TRUE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.undoCount(), 1u);
}

TEST_F(CommandHistoryTest, UndoCommand) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    EXPECT_EQ(testValue, 1);

    history.undo();
    EXPECT_EQ(testValue, 0);
    EXPECT_FALSE(history.canUndo());
    EXPECT_TRUE(history.canRedo());
}

TEST_F(CommandHistoryTest, RedoCommand) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.undo();
    history.redo();

    EXPECT_EQ(testValue, 1);
    EXPECT_TRUE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
}

TEST_F(CommandHistoryTest, NewCommandClearsRedoStack) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.undo();
    EXPECT_TRUE(history.canRedo());

    history.execute(std::make_unique<IncrementCommand>(testValue));
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.redoCount(), 0u);
}

TEST_F(CommandHistoryTest, MultipleUndoRedo) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.execute(std::make_unique<IncrementCommand>(testValue));
    EXPECT_EQ(testValue, 3);

    history.undo();
    history.undo();
    EXPECT_EQ(testValue, 1);
    EXPECT_EQ(history.undoCount(), 1u);
    EXPECT_EQ(history.redoCount(), 2u);

    history.redo();
    EXPECT_EQ(testValue, 2);
}

TEST_F(CommandHistoryTest, Clear) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.execute(std::make_unique<IncrementCommand>(testValue));
    history.undo();

    history.clear();
    EXPECT_FALSE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(history.undoCount(), 0u);
    EXPECT_EQ(history.redoCount(), 0u);
}

TEST_F(CommandHistoryTest, LastExecuted) {
    history.execute(std::make_unique<IncrementCommand>(testValue));
    ASSERT_NE(history.lastExecuted(), nullptr);
    EXPECT_EQ(history.lastExecuted()->name(), "Increment");
}

TEST_F(CommandHistoryTest, SetValueCommandUndoRedo) {
    testValue = 10;
    history.execute(std::make_unique<SetValueCommand>(testValue, 42));
    EXPECT_EQ(testValue, 42);

    history.undo();
    EXPECT_EQ(testValue, 10);

    history.redo();
    EXPECT_EQ(testValue, 42);
}
