/**
 * @file BlueprintGraphTests.cpp
 * @brief Tests for BlueprintNode, BlueprintGraph, and BlueprintEditor
 */

#include <gtest/gtest.h>
#include "blueprint/BlueprintNode.h"
#include "blueprint/BlueprintGraph.h"
#include "blueprint/BlueprintEditor.h"

namespace fresh {

// ============================================================================
// BlueprintNode Tests
// ============================================================================

class BlueprintNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        node = std::make_unique<BlueprintNode>(1, "TestNode", BlueprintNodeType::Function);
    }
    std::unique_ptr<BlueprintNode> node;
};

TEST_F(BlueprintNodeTest, Constructor_SetsIdNameType) {
    EXPECT_EQ(node->getId(), 1);
    EXPECT_EQ(node->getName(), "TestNode");
    EXPECT_EQ(node->getType(), BlueprintNodeType::Function);
}

TEST_F(BlueprintNodeTest, DefaultPosition_IsZero) {
    EXPECT_FLOAT_EQ(node->getPosition().x, 0.0f);
    EXPECT_FLOAT_EQ(node->getPosition().y, 0.0f);
}

TEST_F(BlueprintNodeTest, SetPosition_Updates) {
    node->setPosition(glm::vec2(100.0f, 200.0f));
    EXPECT_FLOAT_EQ(node->getPosition().x, 100.0f);
    EXPECT_FLOAT_EQ(node->getPosition().y, 200.0f);
}

TEST_F(BlueprintNodeTest, AddInput_CreatesPin) {
    BlueprintPin& pin = node->addInput("Value", BlueprintPinType::Float);
    EXPECT_EQ(pin.name, "Value");
    EXPECT_EQ(pin.type, BlueprintPinType::Float);
    EXPECT_FALSE(pin.isOutput);
    EXPECT_EQ(node->getInputs().size(), 1u);
}

TEST_F(BlueprintNodeTest, AddOutput_CreatesPin) {
    BlueprintPin& pin = node->addOutput("Result", BlueprintPinType::Bool);
    EXPECT_EQ(pin.name, "Result");
    EXPECT_EQ(pin.type, BlueprintPinType::Bool);
    EXPECT_TRUE(pin.isOutput);
    EXPECT_EQ(node->getOutputs().size(), 1u);
}

TEST_F(BlueprintNodeTest, PinIds_AreUnique) {
    node->addInput("A", BlueprintPinType::Int);
    node->addInput("B", BlueprintPinType::Int);
    node->addOutput("C", BlueprintPinType::Int);
    int id0 = node->getInputs()[0].id;
    int id1 = node->getInputs()[1].id;
    int id2 = node->getOutputs()[0].id;
    EXPECT_NE(id0, id1);
    EXPECT_NE(id0, id2);
    EXPECT_NE(id1, id2);
}

TEST_F(BlueprintNodeTest, Execute_DefaultReturnsTrue) {
    EXPECT_TRUE(node->execute());
}

// ============================================================================
// BlueprintGraph Tests
// ============================================================================

class BlueprintGraphTest : public ::testing::Test {
protected:
    void SetUp() override {
        graph = std::make_unique<BlueprintGraph>();
        graph->setName("TestGraph");
    }
    std::unique_ptr<BlueprintGraph> graph;
};

TEST_F(BlueprintGraphTest, SetName_Updates) {
    EXPECT_EQ(graph->getName(), "TestGraph");
}

TEST_F(BlueprintGraphTest, AddNode_ReturnsNonNull) {
    BlueprintNode* node = graph->addNode("MyNode", BlueprintNodeType::Event);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->getName(), "MyNode");
    EXPECT_EQ(node->getType(), BlueprintNodeType::Event);
}

TEST_F(BlueprintGraphTest, AddNode_IncreasesNodeCount) {
    graph->addNode("A", BlueprintNodeType::Event);
    graph->addNode("B", BlueprintNodeType::Function);
    EXPECT_EQ(graph->getNodes().size(), 2u);
}

TEST_F(BlueprintGraphTest, GetNode_ById) {
    BlueprintNode* node = graph->addNode("Target", BlueprintNodeType::Branch);
    BlueprintNode* found = graph->getNode(node->getId());
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->getName(), "Target");
}

TEST_F(BlueprintGraphTest, GetNode_InvalidId_ReturnsNull) {
    EXPECT_EQ(graph->getNode(999), nullptr);
}

TEST_F(BlueprintGraphTest, RemoveNode_Success) {
    BlueprintNode* node = graph->addNode("ToRemove", BlueprintNodeType::Custom);
    int id = node->getId();
    EXPECT_TRUE(graph->removeNode(id));
    EXPECT_EQ(graph->getNode(id), nullptr);
    EXPECT_EQ(graph->getNodes().size(), 0u);
}

TEST_F(BlueprintGraphTest, RemoveNode_InvalidId_ReturnsFalse) {
    EXPECT_FALSE(graph->removeNode(999));
}

TEST_F(BlueprintGraphTest, Connect_CreatesConnection) {
    BlueprintNode* a = graph->addNode("A", BlueprintNodeType::Event);
    BlueprintNode* b = graph->addNode("B", BlueprintNodeType::Function);
    a->addOutput("Out", BlueprintPinType::Execution);
    b->addInput("In", BlueprintPinType::Execution);

    EXPECT_TRUE(graph->connect(a->getId(), a->getOutputs()[0].id,
                                b->getId(), b->getInputs()[0].id));
    EXPECT_EQ(graph->getConnections().size(), 1u);
}

TEST_F(BlueprintGraphTest, Disconnect_RemovesConnection) {
    BlueprintNode* a = graph->addNode("A", BlueprintNodeType::Event);
    BlueprintNode* b = graph->addNode("B", BlueprintNodeType::Function);
    a->addOutput("Out", BlueprintPinType::Execution);
    b->addInput("In", BlueprintPinType::Execution);

    int fromPin = a->getOutputs()[0].id;
    int toPin = b->getInputs()[0].id;
    graph->connect(a->getId(), fromPin, b->getId(), toPin);
    EXPECT_TRUE(graph->disconnect(a->getId(), fromPin, b->getId(), toPin));
    EXPECT_EQ(graph->getConnections().size(), 0u);
}

TEST_F(BlueprintGraphTest, Clear_RemovesEverything) {
    graph->addNode("A", BlueprintNodeType::Event);
    graph->addNode("B", BlueprintNodeType::Function);
    graph->clear();
    EXPECT_EQ(graph->getNodes().size(), 0u);
    EXPECT_EQ(graph->getConnections().size(), 0u);
}

TEST_F(BlueprintGraphTest, RemoveNode_AlsoRemovesConnections) {
    BlueprintNode* a = graph->addNode("A", BlueprintNodeType::Event);
    BlueprintNode* b = graph->addNode("B", BlueprintNodeType::Function);
    a->addOutput("Out", BlueprintPinType::Execution);
    b->addInput("In", BlueprintPinType::Execution);
    graph->connect(a->getId(), a->getOutputs()[0].id,
                   b->getId(), b->getInputs()[0].id);
    
    graph->removeNode(a->getId());
    EXPECT_EQ(graph->getConnections().size(), 0u);
}

// ============================================================================
// BlueprintEditor Tests
// ============================================================================

class BlueprintEditorTest : public ::testing::Test {
protected:
    void SetUp() override {
        editor = std::make_unique<BlueprintEditor>();
    }
    std::unique_ptr<BlueprintEditor> editor;
};

TEST_F(BlueprintEditorTest, Initialize_CreatesDefaultGraph) {
    EXPECT_TRUE(editor->initialize());
    EXPECT_NE(editor->getGraph(), nullptr);
}

TEST_F(BlueprintEditorTest, DefaultState_NotVisible) {
    EXPECT_FALSE(editor->isVisible());
}

TEST_F(BlueprintEditorTest, Toggle_ChangesVisibility) {
    editor->toggle();
    EXPECT_TRUE(editor->isVisible());
    editor->toggle();
    EXPECT_FALSE(editor->isVisible());
}

TEST_F(BlueprintEditorTest, NewGraph_ReplacesExisting) {
    editor->initialize();
    editor->getGraph()->addNode("OldNode", BlueprintNodeType::Event);
    editor->newGraph("NewGraph");
    EXPECT_EQ(editor->getGraph()->getName(), "NewGraph");
    EXPECT_EQ(editor->getGraph()->getNodes().size(), 0u);
}

TEST_F(BlueprintEditorTest, Shutdown_CleansUp) {
    editor->initialize();
    editor->shutdown();
    EXPECT_EQ(editor->getGraph(), nullptr);
}

} // namespace fresh
