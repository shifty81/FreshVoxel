#include <gtest/gtest.h>
#include "mod/ModLoader.h"
#include <filesystem>
#include <fstream>

class ModLoaderTest : public ::testing::Test {
protected:
    fresh::ModLoader loader;
};

TEST_F(ModLoaderTest, InitialState) {
    EXPECT_EQ(loader.modCount(), 0u);
}

TEST_F(ModLoaderTest, RegisterMod) {
    fresh::ModDescriptor desc;
    desc.id = "test-mod";
    desc.name = "Test Mod";
    desc.version = "1.0.0";

    auto result = loader.registerMod(desc);
    EXPECT_EQ(result, fresh::ModLoadResult::Success);
    EXPECT_EQ(loader.modCount(), 1u);
    EXPECT_TRUE(loader.hasMod("test-mod"));
}

TEST_F(ModLoaderTest, RegisterInvalidDescriptor) {
    fresh::ModDescriptor desc;
    // Empty id and name
    auto result = loader.registerMod(desc);
    EXPECT_EQ(result, fresh::ModLoadResult::InvalidDescriptor);
}

TEST_F(ModLoaderTest, RegisterDuplicate) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    loader.registerMod(desc);

    auto result = loader.registerMod(desc);
    EXPECT_EQ(result, fresh::ModLoadResult::AlreadyLoaded);
}

TEST_F(ModLoaderTest, UnregisterMod) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    loader.registerMod(desc);

    EXPECT_TRUE(loader.unregisterMod("mod-a"));
    EXPECT_FALSE(loader.hasMod("mod-a"));
    EXPECT_EQ(loader.modCount(), 0u);
}

TEST_F(ModLoaderTest, UnregisterNonexistent) {
    EXPECT_FALSE(loader.unregisterMod("nonexistent"));
}

TEST_F(ModLoaderTest, EnableDisableMod) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    loader.registerMod(desc);

    EXPECT_TRUE(loader.enableMod("mod-a"));
    auto enabled = loader.enabledMods();
    EXPECT_EQ(enabled.size(), 1u);

    EXPECT_TRUE(loader.disableMod("mod-a"));
    enabled = loader.enabledMods();
    EXPECT_EQ(enabled.size(), 0u);
}

TEST_F(ModLoaderTest, EnableNonexistent) {
    EXPECT_FALSE(loader.enableMod("nonexistent"));
}

TEST_F(ModLoaderTest, GetMod) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    desc.author = "Tester";
    loader.registerMod(desc);

    const auto* mod = loader.getMod("mod-a");
    ASSERT_NE(mod, nullptr);
    EXPECT_EQ(mod->author, "Tester");
}

TEST_F(ModLoaderTest, GetModNonexistent) {
    EXPECT_EQ(loader.getMod("nonexistent"), nullptr);
}

TEST_F(ModLoaderTest, DependencyValidation) {
    fresh::ModDescriptor base;
    base.id = "base";
    base.name = "Base";
    loader.registerMod(base);

    fresh::ModDescriptor dependent;
    dependent.id = "dependent";
    dependent.name = "Dependent";
    dependent.dependencies = {"base"};
    loader.registerMod(dependent);

    EXPECT_TRUE(loader.validateDependencies("dependent"));
    EXPECT_TRUE(loader.missingDependencies("dependent").empty());
}

TEST_F(ModLoaderTest, MissingDependency) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    desc.dependencies = {"missing-dep", "another-missing"};
    loader.registerMod(desc);

    EXPECT_FALSE(loader.validateDependencies("mod-a"));
    auto missing = loader.missingDependencies("mod-a");
    EXPECT_EQ(missing.size(), 2u);
}

TEST_F(ModLoaderTest, ScanDirectoryFindsModJson) {
    std::string testDir = "/tmp/test_mod_scan";
    std::filesystem::create_directories(testDir + "/my-mod");
    {
        std::ofstream f(testDir + "/my-mod/mod.json");
        f << "{}";
    }

    size_t found = loader.scanDirectory(testDir);
    EXPECT_EQ(found, 1u);
    EXPECT_TRUE(loader.hasMod("my-mod"));

    // Cleanup
    std::filesystem::remove_all(testDir);
}

TEST_F(ModLoaderTest, ScanNonexistentDirectory) {
    EXPECT_EQ(loader.scanDirectory("/tmp/nonexistent_mod_dir_xyz"), 0u);
}

TEST_F(ModLoaderTest, Clear) {
    fresh::ModDescriptor desc;
    desc.id = "mod-a";
    desc.name = "Mod A";
    loader.registerMod(desc);

    loader.clear();
    EXPECT_EQ(loader.modCount(), 0u);
}
