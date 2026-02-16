#include <gtest/gtest.h>
#include "ai/LLMClient.h"
#include "ai/ProjectContext.h"

using namespace fresh;

/**
 * @brief Test suite for LLMClient and ProjectContext
 */
class LLMClientTest : public ::testing::Test
{
protected:
    LLMClient client;
    ProjectContext context;
};

// ============================================================================
// LLMClient configuration tests
// ============================================================================

TEST_F(LLMClientTest, DefaultConfigHasEndpoint)
{
    LLMClient defaultClient;
    // Default LLMConfig has a pre-configured local endpoint (Ollama)
    EXPECT_TRUE(defaultClient.isConfigured());
}

TEST_F(LLMClientTest, ConfigureEndpoint)
{
    LLMConfig config;
    config.endpoint = "http://localhost:11434/v1/chat/completions";
    config.model = "llama3";
    config.temperature = 0.5f;
    config.maxTokens = 1024;
    
    client.configure(config);
    
    EXPECT_EQ(client.getConfig().endpoint, "http://localhost:11434/v1/chat/completions");
    EXPECT_EQ(client.getConfig().model, "llama3");
    EXPECT_FLOAT_EQ(client.getConfig().temperature, 0.5f);
    EXPECT_EQ(client.getConfig().maxTokens, 1024);
}

TEST_F(LLMClientTest, SetSystemPrompt)
{
    client.setSystemPrompt("You are a helpful assistant.");
    EXPECT_EQ(client.getSystemPrompt(), "You are a helpful assistant.");
}

TEST_F(LLMClientTest, HistoryStartsEmpty)
{
    EXPECT_TRUE(client.getHistory().empty());
}

TEST_F(LLMClientTest, ClearHistoryWorks)
{
    client.setSystemPrompt("test");
    client.clearHistory();
    EXPECT_TRUE(client.getHistory().empty());
    // System prompt should be preserved
    EXPECT_EQ(client.getSystemPrompt(), "test");
}

TEST_F(LLMClientTest, NotProcessingInitially)
{
    EXPECT_FALSE(client.isProcessing());
}

// ============================================================================
// Request body building tests
// ============================================================================

TEST_F(LLMClientTest, BuildRequestBodySingleMessage)
{
    LLMConfig config;
    config.model = "gpt-4";
    config.temperature = 0.7f;
    config.maxTokens = 2048;
    client.configure(config);
    
    std::vector<LLMMessage> messages = {
        {LLMMessage::Role::User, "Hello"}
    };
    
    std::string body = client.buildRequestBody(messages);
    
    // Verify JSON structure
    EXPECT_NE(body.find("\"model\":\"gpt-4\""), std::string::npos);
    EXPECT_NE(body.find("\"role\":\"user\""), std::string::npos);
    EXPECT_NE(body.find("\"content\":\"Hello\""), std::string::npos);
    EXPECT_NE(body.find("\"temperature\":0.7"), std::string::npos);
    EXPECT_NE(body.find("\"max_tokens\":2048"), std::string::npos);
}

TEST_F(LLMClientTest, BuildRequestBodyWithSystemPrompt)
{
    LLMConfig config;
    config.model = "llama3";
    client.configure(config);
    
    std::vector<LLMMessage> messages = {
        {LLMMessage::Role::System, "You are helpful."},
        {LLMMessage::Role::User, "Hi"}
    };
    
    std::string body = client.buildRequestBody(messages);
    
    EXPECT_NE(body.find("\"role\":\"system\""), std::string::npos);
    EXPECT_NE(body.find("\"content\":\"You are helpful.\""), std::string::npos);
    EXPECT_NE(body.find("\"role\":\"user\""), std::string::npos);
}

TEST_F(LLMClientTest, BuildRequestBodyEscapesSpecialChars)
{
    LLMConfig config;
    config.model = "test";
    client.configure(config);
    
    std::vector<LLMMessage> messages = {
        {LLMMessage::Role::User, "Line1\nLine2\t\"quoted\""}
    };
    
    std::string body = client.buildRequestBody(messages);
    
    // Should contain escaped characters
    EXPECT_NE(body.find("\\n"), std::string::npos);
    EXPECT_NE(body.find("\\t"), std::string::npos);
    EXPECT_NE(body.find("\\\"quoted\\\""), std::string::npos);
}

// ============================================================================
// Response parsing tests
// ============================================================================

TEST_F(LLMClientTest, ParseValidResponse)
{
    std::string json = R"({
        "choices": [{
            "message": {
                "role": "assistant",
                "content": "Hello! How can I help?"
            }
        }]
    })";
    
    LLMResponse response = LLMClient::parseResponse(json);
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.content, "Hello! How can I help?");
}

TEST_F(LLMClientTest, ParseErrorResponse)
{
    std::string json = R"({
        "error": {
            "message": "Invalid API key",
            "type": "authentication_error"
        }
    })";
    
    LLMResponse response = LLMClient::parseResponse(json);
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error, "Invalid API key");
}

TEST_F(LLMClientTest, ParseEmptyResponse)
{
    LLMResponse response = LLMClient::parseResponse("");
    EXPECT_FALSE(response.success);
}

TEST_F(LLMClientTest, ParseResponseWithEscapedContent)
{
    std::string json = R"({"choices":[{"message":{"content":"Use `print(\"hello\")`"}}]})";
    
    LLMResponse response = LLMClient::parseResponse(json);
    EXPECT_TRUE(response.success);
    EXPECT_NE(response.content.find("print"), std::string::npos);
}

// ============================================================================
// LLMMessage role conversion tests
// ============================================================================

TEST_F(LLMClientTest, RoleToString)
{
    EXPECT_STREQ(LLMMessage::roleToString(LLMMessage::Role::System), "system");
    EXPECT_STREQ(LLMMessage::roleToString(LLMMessage::Role::User), "user");
    EXPECT_STREQ(LLMMessage::roleToString(LLMMessage::Role::Assistant), "assistant");
}

// ============================================================================
// ProjectContext tests
// ============================================================================

class ProjectContextTest : public ::testing::Test
{
protected:
    ProjectContext context;
};

TEST_F(ProjectContextTest, GeneralPromptContainsEngineInfo)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::General);
    
    // Should contain core engine description
    EXPECT_NE(prompt.find("FreshVoxel Engine"), std::string::npos);
    EXPECT_NE(prompt.find("C++20"), std::string::npos);
    EXPECT_NE(prompt.find("DirectX"), std::string::npos);
}

TEST_F(ProjectContextTest, GeneralPromptContainsAllTopics)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::General);
    
    // General should include all topic contexts
    EXPECT_NE(prompt.find("Lua"), std::string::npos);
    EXPECT_NE(prompt.find("Voxel"), std::string::npos);
    EXPECT_NE(prompt.find("Editor Tools"), std::string::npos);
    EXPECT_NE(prompt.find("NPC"), std::string::npos);
}

TEST_F(ProjectContextTest, LuaTopicContainsScriptingInfo)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::LuaScripting);
    
    EXPECT_NE(prompt.find("Lua"), std::string::npos);
    EXPECT_NE(prompt.find("createEntity"), std::string::npos);
    EXPECT_NE(prompt.find("onUpdate"), std::string::npos);
}

TEST_F(ProjectContextTest, VoxelTopicContainsBlockTypes)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::VoxelBuilding);
    
    EXPECT_NE(prompt.find("Stone"), std::string::npos);
    EXPECT_NE(prompt.find("Grass"), std::string::npos);
    EXPECT_NE(prompt.find("Chunk"), std::string::npos);
}

TEST_F(ProjectContextTest, EditorToolsTopicContainsTools)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::EditorTools);
    
    EXPECT_NE(prompt.find("Brush"), std::string::npos);
    EXPECT_NE(prompt.find("Flatten"), std::string::npos);
    EXPECT_NE(prompt.find("Scene Hierarchy"), std::string::npos);
}

TEST_F(ProjectContextTest, NPCTopicContainsBehaviorInfo)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::NPCAndAI);
    
    EXPECT_NE(prompt.find("Behavior"), std::string::npos);
    EXPECT_NE(prompt.find("Sequence"), std::string::npos);
    EXPECT_NE(prompt.find("Patrol"), std::string::npos);
}

TEST_F(ProjectContextTest, TopicToStringWorks)
{
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::General), "General");
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::LuaScripting), "Lua Scripting");
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::VoxelBuilding), "Voxel Building");
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::EditorTools), "Editor Tools");
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::NPCAndAI), "NPC & AI");
    EXPECT_EQ(ProjectContext::topicToString(ProjectContext::Topic::GameDesign), "Game Design");
}

TEST_F(ProjectContextTest, GetTopicNamesReturnsAllTopics)
{
    auto names = ProjectContext::getTopicNames();
    EXPECT_EQ(names.size(), 6u);
    EXPECT_EQ(names[0], "General");
    EXPECT_EQ(names[5], "Game Design");
}

TEST_F(ProjectContextTest, EngineDescriptionNotEmpty)
{
    std::string desc = ProjectContext::getEngineDescription();
    EXPECT_FALSE(desc.empty());
    EXPECT_NE(desc.find("FreshVoxel Engine"), std::string::npos);
}

TEST_F(ProjectContextTest, GameDesignTopicContainsMultipleContexts)
{
    std::string prompt = context.generateSystemPrompt(ProjectContext::Topic::GameDesign);
    
    // Game design should include voxel building + NPC/AI + Lua scripting
    EXPECT_NE(prompt.find("Block"), std::string::npos);
    EXPECT_NE(prompt.find("NPC"), std::string::npos);
    EXPECT_NE(prompt.find("Lua"), std::string::npos);
}
