#include <gtest/gtest.h>
#include "ecs/DeltaEditStore.h"

class DeltaEditStoreTest : public ::testing::Test {
protected:
    fresh::ecs::DeltaEditStore store{12345};
};

TEST_F(DeltaEditStoreTest, DefaultSeed) {
    EXPECT_EQ(store.seed(), 12345u);
}

TEST_F(DeltaEditStoreTest, RecordAndCount) {
    EXPECT_EQ(store.count(), 0u);
    fresh::ecs::DeltaEdit edit;
    edit.type = fresh::ecs::DeltaEditType::AddObject;
    edit.entityID = 1;
    edit.objectType = "tree";
    store.record(edit);
    EXPECT_EQ(store.count(), 1u);
}

TEST_F(DeltaEditStoreTest, Clear_RemovesEdits) {
    fresh::ecs::DeltaEdit edit;
    edit.type = fresh::ecs::DeltaEditType::AddObject;
    store.record(edit);
    store.clear();
    EXPECT_EQ(store.count(), 0u);
    EXPECT_EQ(store.seed(), 12345u);
}

TEST_F(DeltaEditStoreTest, SerializeDeserialize_RoundTrip) {
    fresh::ecs::DeltaEdit edit1;
    edit1.type = fresh::ecs::DeltaEditType::AddObject;
    edit1.entityID = 1;
    edit1.objectType = "house";
    edit1.position[0] = 10.0f;
    edit1.position[1] = 20.0f;
    edit1.position[2] = 30.0f;

    fresh::ecs::DeltaEdit edit2;
    edit2.type = fresh::ecs::DeltaEditType::SetProperty;
    edit2.entityID = 2;
    edit2.propertyName = "color";
    edit2.propertyValue = "red";

    store.record(edit1);
    store.record(edit2);

    std::string json = store.serializeToJSON();
    EXPECT_FALSE(json.empty());

    fresh::ecs::DeltaEditStore loaded;
    EXPECT_TRUE(loaded.deserializeFromJSON(json));
    EXPECT_EQ(loaded.seed(), 12345u);
    EXPECT_EQ(loaded.count(), 2u);

    const auto& edits = loaded.edits();
    EXPECT_EQ(edits[0].type, fresh::ecs::DeltaEditType::AddObject);
    EXPECT_EQ(edits[0].entityID, 1u);
    EXPECT_EQ(edits[0].objectType, "house");
    EXPECT_FLOAT_EQ(edits[0].position[0], 10.0f);

    EXPECT_EQ(edits[1].type, fresh::ecs::DeltaEditType::SetProperty);
    EXPECT_EQ(edits[1].propertyName, "color");
    EXPECT_EQ(edits[1].propertyValue, "red");
}

TEST_F(DeltaEditStoreTest, DeltaEditTypeName_ReturnsCorrectStrings) {
    EXPECT_STREQ(fresh::ecs::deltaEditTypeName(fresh::ecs::DeltaEditType::AddObject), "AddObject");
    EXPECT_STREQ(fresh::ecs::deltaEditTypeName(fresh::ecs::DeltaEditType::RemoveObject), "RemoveObject");
    EXPECT_STREQ(fresh::ecs::deltaEditTypeName(fresh::ecs::DeltaEditType::MoveObject), "MoveObject");
    EXPECT_STREQ(fresh::ecs::deltaEditTypeName(fresh::ecs::DeltaEditType::SetProperty), "SetProperty");
}
