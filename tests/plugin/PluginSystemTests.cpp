#include <gtest/gtest.h>
#include "plugin/PluginSystem.h"

class PluginValidatorTest : public ::testing::Test {
protected:
    fresh::PluginDescriptor makeValid() {
        fresh::PluginDescriptor d;
        d.name = "TestPlugin";
        d.version = "1.0.0";
        d.engineVersion = "0.1.0";
        d.type = "editor-panel";
        d.deterministic = true;
        return d;
    }
};

TEST_F(PluginValidatorTest, ValidPlugin) {
    auto desc = makeValid();
    EXPECT_EQ(fresh::PluginValidator::validate(desc), fresh::PluginLoadResult::Success);
}

TEST_F(PluginValidatorTest, EmptyName) {
    auto desc = makeValid();
    desc.name = "";
    EXPECT_EQ(fresh::PluginValidator::validate(desc), fresh::PluginLoadResult::InvalidDescriptor);
}

TEST_F(PluginValidatorTest, EmptyVersion) {
    auto desc = makeValid();
    desc.version = "";
    EXPECT_EQ(fresh::PluginValidator::validate(desc), fresh::PluginLoadResult::InvalidDescriptor);
}

TEST_F(PluginValidatorTest, NonDeterministic) {
    auto desc = makeValid();
    desc.deterministic = false;
    EXPECT_EQ(fresh::PluginValidator::validate(desc), fresh::PluginLoadResult::NonDeterministic);
}

TEST_F(PluginValidatorTest, IncompatibleVersion) {
    auto desc = makeValid();
    desc.engineVersion = "1.0.0"; // engine is 0.1.0, major mismatch
    EXPECT_EQ(fresh::PluginValidator::validate(desc, "0.1.0"), fresh::PluginLoadResult::IncompatibleVersion);
}

TEST_F(PluginValidatorTest, VersionCompatibility) {
    EXPECT_TRUE(fresh::PluginValidator::isVersionCompatible("0.1.0", "0.2.0"));
    EXPECT_FALSE(fresh::PluginValidator::isVersionCompatible("1.0.0", "0.2.0"));
    EXPECT_FALSE(fresh::PluginValidator::isVersionCompatible("", "0.1.0"));
}

class PluginRegistryTest : public ::testing::Test {
protected:
    fresh::PluginRegistry registry;

    fresh::PluginDescriptor makePlugin(const std::string& name, const std::string& type = "editor-panel") {
        fresh::PluginDescriptor d;
        d.name = name;
        d.version = "0.1.0";
        d.engineVersion = "0.1.0";
        d.type = type;
        d.deterministic = true;
        return d;
    }
};

TEST_F(PluginRegistryTest, RegisterAndFind) {
    auto desc = makePlugin("MyPlugin");
    EXPECT_TRUE(registry.registerPlugin(desc));
    EXPECT_EQ(registry.pluginCount(), 1u);

    const auto* found = registry.findPlugin("MyPlugin");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->name, "MyPlugin");
}

TEST_F(PluginRegistryTest, RegisterDuplicate) {
    auto desc = makePlugin("MyPlugin");
    EXPECT_TRUE(registry.registerPlugin(desc));
    EXPECT_FALSE(registry.registerPlugin(desc));
    EXPECT_EQ(registry.pluginCount(), 1u);
}

TEST_F(PluginRegistryTest, RegisterInvalid) {
    fresh::PluginDescriptor desc;
    // Empty name/version
    EXPECT_FALSE(registry.registerPlugin(desc));
}

TEST_F(PluginRegistryTest, Unregister) {
    registry.registerPlugin(makePlugin("A"));
    registry.registerPlugin(makePlugin("B"));
    EXPECT_EQ(registry.pluginCount(), 2u);

    EXPECT_TRUE(registry.unregisterPlugin("A"));
    EXPECT_EQ(registry.pluginCount(), 1u);
    EXPECT_EQ(registry.findPlugin("A"), nullptr);
}

TEST_F(PluginRegistryTest, UnregisterNonexistent) {
    EXPECT_FALSE(registry.unregisterPlugin("nope"));
}

TEST_F(PluginRegistryTest, GetAll) {
    registry.registerPlugin(makePlugin("A"));
    registry.registerPlugin(makePlugin("B"));

    auto all = registry.getAll();
    EXPECT_EQ(all.size(), 2u);
}

TEST_F(PluginRegistryTest, GetByType) {
    registry.registerPlugin(makePlugin("Editor1", "editor-panel"));
    registry.registerPlugin(makePlugin("Importer1", "asset-importer"));
    registry.registerPlugin(makePlugin("Editor2", "editor-panel"));

    auto editors = registry.getByType("editor-panel");
    EXPECT_EQ(editors.size(), 2u);

    auto importers = registry.getByType("asset-importer");
    EXPECT_EQ(importers.size(), 1u);
}
