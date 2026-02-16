#include <gtest/gtest.h>
#include "schema/SchemaValidator.h"

class SchemaValidatorTest : public ::testing::Test {
protected:
    fresh::SchemaValidator validator;

    fresh::SchemaDefinition makeValid() {
        fresh::SchemaDefinition schema;
        schema.id = "test.schema";
        schema.version = 1;
        return schema;
    }
};

TEST_F(SchemaValidatorTest, ValidEmptySchema) {
    auto schema = makeValid();
    EXPECT_TRUE(validator.validate(schema));
    EXPECT_TRUE(validator.getErrors().empty());
}

TEST_F(SchemaValidatorTest, EmptyId) {
    auto schema = makeValid();
    schema.id = "";
    EXPECT_FALSE(validator.validate(schema));
    EXPECT_FALSE(validator.getErrors().empty());
}

TEST_F(SchemaValidatorTest, InvalidVersion) {
    auto schema = makeValid();
    schema.version = 0;
    EXPECT_FALSE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, NegativeVersion) {
    auto schema = makeValid();
    schema.version = -1;
    EXPECT_FALSE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, ValidNodesPass) {
    auto schema = makeValid();
    fresh::SchemaNodeDef node;
    node.id = "node1";
    node.inputs.push_back({"x", fresh::SchemaValueType::Float, true});
    node.outputs.push_back({"result", fresh::SchemaValueType::Float, true});
    schema.nodes.push_back(node);

    EXPECT_TRUE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, EmptyNodeId) {
    auto schema = makeValid();
    fresh::SchemaNodeDef node;
    node.id = ""; // invalid
    schema.nodes.push_back(node);

    EXPECT_FALSE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, DuplicateNodeIds) {
    auto schema = makeValid();
    fresh::SchemaNodeDef node1;
    node1.id = "dup";
    fresh::SchemaNodeDef node2;
    node2.id = "dup";
    schema.nodes.push_back(node1);
    schema.nodes.push_back(node2);

    EXPECT_FALSE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, EmptyFieldName) {
    auto schema = makeValid();
    fresh::SchemaNodeDef node;
    node.id = "node1";
    node.inputs.push_back({"", fresh::SchemaValueType::Int, true}); // empty name
    schema.nodes.push_back(node);

    EXPECT_FALSE(validator.validate(schema));
}

TEST_F(SchemaValidatorTest, ClearResets) {
    auto schema = makeValid();
    schema.id = "";
    validator.validate(schema);
    EXPECT_FALSE(validator.getErrors().empty());

    validator.clear();
    EXPECT_TRUE(validator.getErrors().empty());
}
