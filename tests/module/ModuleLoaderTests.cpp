#include <gtest/gtest.h>
#include "module/ModuleLoader.h"

namespace {

class TestModule : public fresh::IGameModule {
public:
    int startCount = 0;
    int tickCount = 0;
    int shutdownCount = 0;
    float lastDt = 0.0f;

    fresh::GameModuleDesc describe() const override {
        return {"TestModule", 1};
    }

    void onStart() override { startCount++; }
    void onTick(float dt) override { tickCount++; lastDt = dt; }
    void onShutdown() override { shutdownCount++; }
};

} // namespace

class ModuleLoaderTest : public ::testing::Test {
protected:
    fresh::ModuleLoader loader;
};

TEST_F(ModuleLoaderTest, InitialState) {
    EXPECT_FALSE(loader.isLoaded());
    EXPECT_EQ(loader.getModule(), nullptr);
}

TEST_F(ModuleLoaderTest, SetStaticModule) {
    auto mod = std::make_unique<TestModule>();
    auto* rawPtr = mod.get();

    loader.setStaticModule(std::move(mod));
    EXPECT_TRUE(loader.isLoaded());
    EXPECT_EQ(loader.getModule(), rawPtr);
}

TEST_F(ModuleLoaderTest, StaticModuleLifecycle) {
    auto mod = std::make_unique<TestModule>();
    auto* raw = mod.get();
    loader.setStaticModule(std::move(mod));

    auto desc = raw->describe();
    EXPECT_STREQ(desc.name, "TestModule");
    EXPECT_EQ(desc.version, 1u);

    raw->onStart();
    EXPECT_EQ(raw->startCount, 1);

    raw->onTick(0.016f);
    EXPECT_EQ(raw->tickCount, 1);
    EXPECT_FLOAT_EQ(raw->lastDt, 0.016f);

    raw->onShutdown();
    EXPECT_EQ(raw->shutdownCount, 1);
}

TEST_F(ModuleLoaderTest, Unload) {
    loader.setStaticModule(std::make_unique<TestModule>());
    EXPECT_TRUE(loader.isLoaded());

    loader.unload();
    EXPECT_FALSE(loader.isLoaded());
    EXPECT_EQ(loader.getModule(), nullptr);
}

TEST_F(ModuleLoaderTest, SetStaticModuleReplacesExisting) {
    auto mod1 = std::make_unique<TestModule>();
    auto mod2 = std::make_unique<TestModule>();
    auto* raw2 = mod2.get();

    loader.setStaticModule(std::move(mod1));
    loader.setStaticModule(std::move(mod2));

    EXPECT_EQ(loader.getModule(), raw2);
}

TEST_F(ModuleLoaderTest, LoadNonexistentLibrary) {
    auto result = loader.load("/tmp/nonexistent_module.so");
    EXPECT_EQ(result, fresh::ModuleLoadResult::NotFound);
    EXPECT_FALSE(loader.isLoaded());
}

TEST_F(ModuleLoaderTest, LoadWhenAlreadyLoaded) {
    loader.setStaticModule(std::make_unique<TestModule>());
    auto result = loader.load("/tmp/some_module.so");
    EXPECT_EQ(result, fresh::ModuleLoadResult::AlreadyLoaded);
}
