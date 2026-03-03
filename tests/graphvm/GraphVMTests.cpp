#include <gtest/gtest.h>
#include "graphvm/GraphVM.h"
#include "graphvm/GraphCompiler.h"
#include "core/EventBus.h"

class GraphVMTest : public ::testing::Test {
protected:
    fresh::vm::GraphVM vm;
};

TEST_F(GraphVMTest, AddConstantsAndAdd) {
    fresh::vm::Bytecode bc;
    bc.constants = {10, 20};
    bc.instructions = {
        {fresh::vm::OpCode::LOAD_CONST, 0, 0, 0},
        {fresh::vm::OpCode::LOAD_CONST, 1, 0, 0},
        {fresh::vm::OpCode::ADD, 0, 0, 0},
        {fresh::vm::OpCode::STORE_VAR, 0, 0, 0},
        {fresh::vm::OpCode::END, 0, 0, 0}
    };

    fresh::vm::VMContext ctx;
    vm.execute(bc, ctx);
    EXPECT_EQ(vm.getLocal(0), 30);
}

TEST_F(GraphVMTest, SubMulDiv) {
    fresh::vm::Bytecode bc;
    bc.constants = {100, 30, 2};
    bc.instructions = {
        {fresh::vm::OpCode::LOAD_CONST, 0, 0, 0},
        {fresh::vm::OpCode::LOAD_CONST, 1, 0, 0},
        {fresh::vm::OpCode::SUB, 0, 0, 0},
        {fresh::vm::OpCode::LOAD_CONST, 2, 0, 0},
        {fresh::vm::OpCode::MUL, 0, 0, 0},
        {fresh::vm::OpCode::STORE_VAR, 0, 0, 0},
        {fresh::vm::OpCode::END, 0, 0, 0}
    };

    fresh::vm::VMContext ctx;
    vm.execute(bc, ctx);
    EXPECT_EQ(vm.getLocal(0), 140);
}

TEST_F(GraphVMTest, DivByZero_ReturnsZero) {
    fresh::vm::Bytecode bc;
    bc.constants = {42, 0};
    bc.instructions = {
        {fresh::vm::OpCode::LOAD_CONST, 0, 0, 0},
        {fresh::vm::OpCode::LOAD_CONST, 1, 0, 0},
        {fresh::vm::OpCode::DIV, 0, 0, 0},
        {fresh::vm::OpCode::STORE_VAR, 0, 0, 0},
        {fresh::vm::OpCode::END, 0, 0, 0}
    };

    fresh::vm::VMContext ctx;
    vm.execute(bc, ctx);
    EXPECT_EQ(vm.getLocal(0), 0);
}

TEST_F(GraphVMTest, Comparison) {
    fresh::vm::Bytecode bc;
    bc.constants = {5, 10};
    bc.instructions = {
        {fresh::vm::OpCode::LOAD_CONST, 0, 0, 0},
        {fresh::vm::OpCode::LOAD_CONST, 1, 0, 0},
        {fresh::vm::OpCode::CMP_LT, 0, 0, 0},
        {fresh::vm::OpCode::STORE_VAR, 0, 0, 0},
        {fresh::vm::OpCode::END, 0, 0, 0}
    };

    fresh::vm::VMContext ctx;
    vm.execute(bc, ctx);
    EXPECT_EQ(vm.getLocal(0), 1);
}

TEST_F(GraphVMTest, EmitEvent_PublishesToBus) {
    fresh::EventBus bus;
    int eventCount = 0;
    bus.subscribe("TEST_EVENT", [&](const fresh::Event& e) {
        eventCount++;
        EXPECT_EQ(e.intParam, 42);
    });

    fresh::vm::Bytecode bc;
    bc.constants = {42};
    bc.eventNames = {"TEST_EVENT"};
    bc.instructions = {
        {fresh::vm::OpCode::LOAD_CONST, 0, 0, 0},
        {fresh::vm::OpCode::EMIT_EVENT, 0, 0, 0},
        {fresh::vm::OpCode::END, 0, 0, 0}
    };

    fresh::vm::VMContext ctx;
    ctx.eventBus = &bus;
    ctx.entity = 7;
    vm.execute(bc, ctx);

    EXPECT_EQ(eventCount, 1);
}

TEST_F(GraphVMTest, GraphCompiler_CompilesAndRuns) {
    fresh::graph::Graph graph;
    graph.nodes = {
        {0, fresh::graph::NodeType::Constant, 5},
        {1, fresh::graph::NodeType::Constant, 3},
        {2, fresh::graph::NodeType::Add, 0}
    };

    fresh::vm::GraphCompiler compiler;
    auto bc = compiler.compile(graph);
    // Insert STORE_VAR before the compiler-generated END instruction
    bc.instructions.insert(bc.instructions.end() - 1,
        {fresh::vm::OpCode::STORE_VAR, 0, 0, 0});

    fresh::vm::VMContext ctx;
    vm.execute(bc, ctx);
    EXPECT_EQ(vm.getLocal(0), 8);
}
