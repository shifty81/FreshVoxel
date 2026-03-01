/**
 * @file DialogueSystemTests.cpp
 * @brief Tests for DialogueNode, DialogueGraph, and DialogueManager
 */

#include <gtest/gtest.h>
#include "dialogue/DialogueNode.h"
#include "dialogue/DialogueGraph.h"
#include "dialogue/DialogueManager.h"

namespace fresh {

// ============================================================================
// DialogueNode Tests
// ============================================================================

class DialogueNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        node = std::make_unique<DialogueNode>(1, DialogueNodeType::Dialogue);
    }
    std::unique_ptr<DialogueNode> node;
};

TEST_F(DialogueNodeTest, Constructor_SetsIdAndType) {
    EXPECT_EQ(node->getId(), 1);
    EXPECT_EQ(node->getType(), DialogueNodeType::Dialogue);
}

TEST_F(DialogueNodeTest, SetSpeakerName) {
    node->setSpeakerName("NPC_Guard");
    EXPECT_EQ(node->getSpeakerName(), "NPC_Guard");
}

TEST_F(DialogueNodeTest, SetText) {
    node->setText("Hello, traveler!");
    EXPECT_EQ(node->getText(), "Hello, traveler!");
}

TEST_F(DialogueNodeTest, SetPosition) {
    node->setPosition(glm::vec2(50.0f, 75.0f));
    EXPECT_FLOAT_EQ(node->getPosition().x, 50.0f);
    EXPECT_FLOAT_EQ(node->getPosition().y, 75.0f);
}

TEST_F(DialogueNodeTest, SetNextNodeId) {
    node->setNextNodeId(5);
    EXPECT_EQ(node->getNextNodeId(), 5);
}

TEST_F(DialogueNodeTest, DefaultNextNodeId_IsNegativeOne) {
    EXPECT_EQ(node->getNextNodeId(), -1);
}

TEST_F(DialogueNodeTest, AddChoice_CreatesChoice) {
    auto choiceNode = std::make_unique<DialogueNode>(2, DialogueNodeType::Choice);
    DialogueChoice& choice = choiceNode->addChoice("Yes, I'll help");
    EXPECT_EQ(choice.text, "Yes, I'll help");
    EXPECT_EQ(choiceNode->getChoices().size(), 1u);
}

TEST_F(DialogueNodeTest, AddMultipleChoices_UniqueIds) {
    auto choiceNode = std::make_unique<DialogueNode>(2, DialogueNodeType::Choice);
    DialogueChoice& c1 = choiceNode->addChoice("Option A");
    DialogueChoice& c2 = choiceNode->addChoice("Option B");
    EXPECT_NE(c1.id, c2.id);
    EXPECT_EQ(choiceNode->getChoices().size(), 2u);
}

TEST_F(DialogueNodeTest, ConditionNode_SetCondition) {
    auto condNode = std::make_unique<DialogueNode>(3, DialogueNodeType::Condition);
    condNode->setCondition("player_gold > 100");
    condNode->setTrueBranchNodeId(10);
    condNode->setFalseBranchNodeId(20);
    EXPECT_EQ(condNode->getCondition(), "player_gold > 100");
    EXPECT_EQ(condNode->getTrueBranchNodeId(), 10);
    EXPECT_EQ(condNode->getFalseBranchNodeId(), 20);
}

TEST_F(DialogueNodeTest, ScriptNode_SetScript) {
    auto scriptNode = std::make_unique<DialogueNode>(4, DialogueNodeType::LuaScript);
    scriptNode->setScript("player:addGold(50)");
    EXPECT_EQ(scriptNode->getScript(), "player:addGold(50)");
}

// ============================================================================
// DialogueGraph Tests
// ============================================================================

class DialogueGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<DialogueGraph>();
        graph->setName("TestDialogue");
    }
    std::unique_ptr<DialogueGraph> graph;
};

TEST_F(DialogueGraphTest, SetName_Updates) {
    EXPECT_EQ(graph->getName(), "TestDialogue");
}

TEST_F(DialogueGraphTest, AddNode_ReturnsNonNull) {
    DialogueNode* node = graph->addNode(DialogueNodeType::Start);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getType(), DialogueNodeType::Start);
}

TEST_F(DialogueGraphTest, AddNode_IncreasesCount) {
    graph->addNode(DialogueNodeType::Start);
    graph->addNode(DialogueNodeType::Dialogue);
    graph->addNode(DialogueNodeType::End);
    EXPECT_EQ(graph->getNodes().size(), 3u);
}

TEST_F(DialogueGraphTest, GetNode_ById) {
    DialogueNode* node = graph->addNode(DialogueNodeType::Dialogue);
    ASSERT_NE(node, nullptr);
    DialogueNode* found = graph->getNode(node->getId());
    EXPECT_EQ(found, node);
}

TEST_F(DialogueGraphTest, GetNode_InvalidId_ReturnsNull) {
    EXPECT_EQ(graph->getNode(999), nullptr);
}

TEST_F(DialogueGraphTest, RemoveNode_Success) {
    DialogueNode* node = graph->addNode(DialogueNodeType::Dialogue);
    int id = node->getId();
    EXPECT_TRUE(graph->removeNode(id));
    EXPECT_EQ(graph->getNode(id), nullptr);
}

TEST_F(DialogueGraphTest, RemoveNode_InvalidId_ReturnsFalse) {
    EXPECT_FALSE(graph->removeNode(999));
}

TEST_F(DialogueGraphTest, SetStartNode) {
    DialogueNode* start = graph->addNode(DialogueNodeType::Start);
    graph->setStartNode(start->getId());
    EXPECT_EQ(graph->getStartNode(), start);
}

TEST_F(DialogueGraphTest, GetStartNode_WhenNotSet_ReturnsNull) {
    EXPECT_EQ(graph->getStartNode(), nullptr);
}

TEST_F(DialogueGraphTest, Clear_RemovesEverything) {
    graph->addNode(DialogueNodeType::Start);
    graph->addNode(DialogueNodeType::Dialogue);
    graph->clear();
    EXPECT_EQ(graph->getNodes().size(), 0u);
}

// ============================================================================
// DialogueManager Tests
// ============================================================================

class DialogueManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<DialogueManager>();
        graph = std::make_unique<DialogueGraph>();
        graph->setName("TestDialogue");
    }

    // Build a simple linear dialogue: Start -> Dialogue1 -> Dialogue2 -> End
    void buildSimpleDialogue() {
        DialogueNode* start = graph->addNode(DialogueNodeType::Start);
        DialogueNode* d1 = graph->addNode(DialogueNodeType::Dialogue);
        DialogueNode* d2 = graph->addNode(DialogueNodeType::Dialogue);
        DialogueNode* end = graph->addNode(DialogueNodeType::End);

        d1->setSpeakerName("Guard");
        d1->setText("Halt! Who goes there?");
        d2->setSpeakerName("Guard");
        d2->setText("Very well, you may pass.");

        start->setNextNodeId(d1->getId());
        d1->setNextNodeId(d2->getId());
        d2->setNextNodeId(end->getId());

        graph->setStartNode(start->getId());
    }

    // Build a dialogue with choices: Start -> Dialogue -> Choice -> (two Dialogues) -> End
    void buildChoiceDialogue() {
        DialogueNode* start = graph->addNode(DialogueNodeType::Start);
        DialogueNode* greeting = graph->addNode(DialogueNodeType::Dialogue);
        DialogueNode* choice = graph->addNode(DialogueNodeType::Choice);
        DialogueNode* accept = graph->addNode(DialogueNodeType::Dialogue);
        DialogueNode* decline = graph->addNode(DialogueNodeType::Dialogue);
        DialogueNode* end = graph->addNode(DialogueNodeType::End);

        greeting->setSpeakerName("Merchant");
        greeting->setText("Would you like to trade?");
        accept->setSpeakerName("Merchant");
        accept->setText("Excellent! Let's trade.");
        decline->setSpeakerName("Merchant");
        decline->setText("Perhaps another time.");

        start->setNextNodeId(greeting->getId());
        greeting->setNextNodeId(choice->getId());

        DialogueChoice& c1 = choice->addChoice("Yes, let's trade");
        c1.targetNodeId = accept->getId();
        DialogueChoice& c2 = choice->addChoice("No thanks");
        c2.targetNodeId = decline->getId();

        accept->setNextNodeId(end->getId());
        decline->setNextNodeId(end->getId());

        graph->setStartNode(start->getId());
    }

    std::unique_ptr<DialogueManager> manager;
    std::unique_ptr<DialogueGraph> graph;
};

TEST_F(DialogueManagerTest, DefaultState_Inactive) {
    EXPECT_EQ(manager->getState(), DialogueState::Inactive);
    EXPECT_FALSE(manager->isActive());
}

TEST_F(DialogueManagerTest, StartDialogue_NullGraph_Fails) {
    EXPECT_FALSE(manager->startDialogue(nullptr));
}

TEST_F(DialogueManagerTest, StartDialogue_NoStartNode_Fails) {
    graph->addNode(DialogueNodeType::Dialogue);
    EXPECT_FALSE(manager->startDialogue(graph.get()));
}

TEST_F(DialogueManagerTest, StartDialogue_Success) {
    buildSimpleDialogue();
    EXPECT_TRUE(manager->startDialogue(graph.get()));
    EXPECT_TRUE(manager->isActive());
    EXPECT_EQ(manager->getState(), DialogueState::ShowingDialogue);
}

TEST_F(DialogueManagerTest, StartDialogue_AdvancesToFirstDialogueNode) {
    buildSimpleDialogue();
    manager->startDialogue(graph.get());
    DialogueNode* current = manager->getCurrentNode();
    ASSERT_NE(current, nullptr);
    EXPECT_EQ(current->getType(), DialogueNodeType::Dialogue);
    EXPECT_EQ(current->getText(), "Halt! Who goes there?");
}

TEST_F(DialogueManagerTest, Advance_MovesToNextNode) {
    buildSimpleDialogue();
    manager->startDialogue(graph.get());
    EXPECT_TRUE(manager->advance());
    DialogueNode* current = manager->getCurrentNode();
    ASSERT_NE(current, nullptr);
    EXPECT_EQ(current->getText(), "Very well, you may pass.");
}

TEST_F(DialogueManagerTest, Advance_ToEnd_CompletesDialogue) {
    buildSimpleDialogue();
    manager->startDialogue(graph.get());
    manager->advance(); // d1 -> d2
    manager->advance(); // d2 -> end
    EXPECT_EQ(manager->getState(), DialogueState::Complete);
    EXPECT_FALSE(manager->isActive());
}

TEST_F(DialogueManagerTest, Advance_WhenInactive_Fails) {
    EXPECT_FALSE(manager->advance());
}

TEST_F(DialogueManagerTest, ChoiceDialogue_ShowsChoices) {
    buildChoiceDialogue();
    manager->startDialogue(graph.get());
    // Advance past greeting to choice
    manager->advance();
    EXPECT_EQ(manager->getState(), DialogueState::ShowingChoices);
}

TEST_F(DialogueManagerTest, SelectChoice_FollowsBranch) {
    buildChoiceDialogue();
    manager->startDialogue(graph.get());
    manager->advance(); // greeting -> choice

    // Get the choice IDs
    DialogueNode* choiceNode = manager->getCurrentNode();
    ASSERT_NE(choiceNode, nullptr);
    const auto& choices = choiceNode->getChoices();
    ASSERT_GE(choices.size(), 2u);

    // Select the first choice ("Yes, let's trade")
    EXPECT_TRUE(manager->selectChoice(choices[0].id));
    EXPECT_EQ(manager->getState(), DialogueState::ShowingDialogue);
    EXPECT_EQ(manager->getCurrentNode()->getText(), "Excellent! Let's trade.");
}

TEST_F(DialogueManagerTest, SelectChoice_SecondOption) {
    buildChoiceDialogue();
    manager->startDialogue(graph.get());
    manager->advance(); // greeting -> choice

    DialogueNode* choiceNode = manager->getCurrentNode();
    const auto& choices = choiceNode->getChoices();

    // Select second choice ("No thanks")
    EXPECT_TRUE(manager->selectChoice(choices[1].id));
    EXPECT_EQ(manager->getCurrentNode()->getText(), "Perhaps another time.");
}

TEST_F(DialogueManagerTest, SelectChoice_InvalidId_Fails) {
    buildChoiceDialogue();
    manager->startDialogue(graph.get());
    manager->advance(); // greeting -> choice
    EXPECT_FALSE(manager->selectChoice(999));
}

TEST_F(DialogueManagerTest, StopDialogue_ResetsState) {
    buildSimpleDialogue();
    manager->startDialogue(graph.get());
    manager->stopDialogue();
    EXPECT_EQ(manager->getState(), DialogueState::Inactive);
    EXPECT_FALSE(manager->isActive());
    EXPECT_EQ(manager->getCurrentNode(), nullptr);
}

// ============================================================================
// Condition Evaluation Tests
// ============================================================================

TEST_F(DialogueManagerTest, ConditionNode_TrueBranch) {
    // Build: Start -> Condition(gold > 50) -> [true: Dialogue("Rich")] [false: Dialogue("Poor")] -> End
    DialogueNode* start = graph->addNode(DialogueNodeType::Start);
    DialogueNode* cond = graph->addNode(DialogueNodeType::Condition);
    DialogueNode* richNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* poorNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* end = graph->addNode(DialogueNodeType::End);

    cond->setCondition("gold > 50");
    cond->setTrueBranchNodeId(richNode->getId());
    cond->setFalseBranchNodeId(poorNode->getId());
    richNode->setText("You are rich!");
    richNode->setNextNodeId(end->getId());
    poorNode->setText("You are poor!");
    poorNode->setNextNodeId(end->getId());
    start->setNextNodeId(cond->getId());
    graph->setStartNode(start->getId());

    // Set variable so condition is true
    manager->setVariable("gold", 100);
    EXPECT_TRUE(manager->startDialogue(graph.get()));
    EXPECT_EQ(manager->getCurrentNode()->getText(), "You are rich!");
}

TEST_F(DialogueManagerTest, ConditionNode_FalseBranch) {
    DialogueNode* start = graph->addNode(DialogueNodeType::Start);
    DialogueNode* cond = graph->addNode(DialogueNodeType::Condition);
    DialogueNode* richNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* poorNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* end = graph->addNode(DialogueNodeType::End);

    cond->setCondition("gold > 50");
    cond->setTrueBranchNodeId(richNode->getId());
    cond->setFalseBranchNodeId(poorNode->getId());
    richNode->setText("You are rich!");
    richNode->setNextNodeId(end->getId());
    poorNode->setText("You are poor!");
    poorNode->setNextNodeId(end->getId());
    start->setNextNodeId(cond->getId());
    graph->setStartNode(start->getId());

    // Set variable so condition is false
    manager->setVariable("gold", 10);
    EXPECT_TRUE(manager->startDialogue(graph.get()));
    EXPECT_EQ(manager->getCurrentNode()->getText(), "You are poor!");
}

TEST_F(DialogueManagerTest, ConditionNode_EqualityOperator) {
    DialogueNode* start = graph->addNode(DialogueNodeType::Start);
    DialogueNode* cond = graph->addNode(DialogueNodeType::Condition);
    DialogueNode* yesNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* noNode = graph->addNode(DialogueNodeType::Dialogue);

    cond->setCondition("quest_complete == 1");
    cond->setTrueBranchNodeId(yesNode->getId());
    cond->setFalseBranchNodeId(noNode->getId());
    yesNode->setText("Quest done!");
    noNode->setText("Not yet.");
    start->setNextNodeId(cond->getId());
    graph->setStartNode(start->getId());

    manager->setVariable("quest_complete", 1);
    EXPECT_TRUE(manager->startDialogue(graph.get()));
    EXPECT_EQ(manager->getCurrentNode()->getText(), "Quest done!");
}

TEST_F(DialogueManagerTest, ConditionNode_UndefinedVariable_IsZero) {
    DialogueNode* start = graph->addNode(DialogueNodeType::Start);
    DialogueNode* cond = graph->addNode(DialogueNodeType::Condition);
    DialogueNode* yesNode = graph->addNode(DialogueNodeType::Dialogue);
    DialogueNode* noNode = graph->addNode(DialogueNodeType::Dialogue);

    cond->setCondition("undefined_var == 0");
    cond->setTrueBranchNodeId(yesNode->getId());
    cond->setFalseBranchNodeId(noNode->getId());
    yesNode->setText("Zero");
    noNode->setText("Not zero");
    start->setNextNodeId(cond->getId());
    graph->setStartNode(start->getId());

    // Don't set undefined_var — should default to 0
    EXPECT_TRUE(manager->startDialogue(graph.get()));
    EXPECT_EQ(manager->getCurrentNode()->getText(), "Zero");
}

TEST_F(DialogueManagerTest, SetAndGetVariable) {
    manager->setVariable("test_var", 42);
    EXPECT_EQ(manager->getVariable("test_var"), 42);
    EXPECT_EQ(manager->getVariable("nonexistent"), 0);
}

} // namespace fresh
