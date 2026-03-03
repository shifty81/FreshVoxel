#include <gtest/gtest.h>
#include "ecs/SystemRegistry.h"

namespace {

class TestSystem : public fresh::ecs::ISystem {
public:
    TestSystem(const char* name, int priority = 100)
        : m_name(name), m_priority(priority) {}

    const char* name() const override { return m_name; }
    int priority() const override { return m_priority; }
    void init() override { initCalled = true; }
    void execute(float dt) override { executeCalled = true; lastDt = dt; executeOrder = ++globalOrder; }
    void shutdown() override { shutdownCalled = true; }

    bool initCalled = false;
    bool executeCalled = false;
    bool shutdownCalled = false;
    float lastDt = 0.0f;
    int executeOrder = 0;

    static inline int globalOrder = 0;

private:
    const char* m_name;
    int m_priority;
};

} // anonymous namespace

class SystemRegistryTest : public ::testing::Test {
protected:
    fresh::ecs::SystemRegistry registry;

    void SetUp() override {
        TestSystem::globalOrder = 0;
    }
};

TEST_F(SystemRegistryTest, Register_IncreasesCount) {
    EXPECT_EQ(registry.count(), 0u);
    registry.registerSystem(std::make_unique<TestSystem>("Test"));
    EXPECT_EQ(registry.count(), 1u);
}

TEST_F(SystemRegistryTest, InitAll_CallsInit) {
    auto sys = std::make_unique<TestSystem>("Test");
    auto* ptr = sys.get();
    registry.registerSystem(std::move(sys));
    registry.initAll();
    EXPECT_TRUE(ptr->initCalled);
}

TEST_F(SystemRegistryTest, UpdateAll_CallsExecute) {
    auto sys = std::make_unique<TestSystem>("Test");
    auto* ptr = sys.get();
    registry.registerSystem(std::move(sys));
    registry.updateAll(0.016f);
    EXPECT_TRUE(ptr->executeCalled);
    EXPECT_FLOAT_EQ(ptr->lastDt, 0.016f);
}

TEST_F(SystemRegistryTest, PriorityOrder_LowerRunsFirst) {
    auto highPrio = std::make_unique<TestSystem>("High", 10);
    auto lowPrio = std::make_unique<TestSystem>("Low", 200);
    auto* highPtr = highPrio.get();
    auto* lowPtr = lowPrio.get();

    registry.registerSystem(std::move(lowPrio));
    registry.registerSystem(std::move(highPrio));
    registry.updateAll(0.016f);

    EXPECT_LT(highPtr->executeOrder, lowPtr->executeOrder);
}

TEST_F(SystemRegistryTest, DisabledSystem_Skipped) {
    auto sys = std::make_unique<TestSystem>("Test");
    auto* ptr = sys.get();
    ptr->setEnabled(false);
    registry.registerSystem(std::move(sys));
    registry.updateAll(0.016f);
    EXPECT_FALSE(ptr->executeCalled);
}

TEST_F(SystemRegistryTest, FindByName_ReturnsCorrectSystem) {
    registry.registerSystem(std::make_unique<TestSystem>("Alpha"));
    registry.registerSystem(std::make_unique<TestSystem>("Beta"));

    auto* found = registry.findByName("Beta");
    EXPECT_NE(found, nullptr);
    EXPECT_STREQ(found->name(), "Beta");

    EXPECT_EQ(registry.findByName("Gamma"), nullptr);
}

TEST_F(SystemRegistryTest, ShutdownAll_CallsShutdown) {
    auto sys = std::make_unique<TestSystem>("Test");
    auto* ptr = sys.get();
    registry.registerSystem(std::move(sys));
    registry.initAll();
    registry.shutdownAll();
    EXPECT_TRUE(ptr->shutdownCalled);
}
