#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>

namespace fresh
{

/**
 * @brief Configuration for connecting to an LLM API endpoint
 * 
 * Supports OpenAI-compatible APIs including:
 * - OpenAI (api.openai.com)
 * - Local LLMs via Ollama (localhost:11434)
 * - LM Studio (localhost:1234)
 * - Any OpenAI-compatible endpoint
 */
struct LLMConfig
{
    std::string endpoint = "http://localhost:11434/v1/chat/completions";
    std::string apiKey;
    std::string model = "llama3";
    float temperature = 0.7f;
    int maxTokens = 2048;
    int timeoutSeconds = 30;
};

/**
 * @brief Represents a single message in an LLM conversation
 */
struct LLMMessage
{
    enum class Role { System, User, Assistant };
    
    Role role;
    std::string content;
    
    static const char* roleToString(Role r)
    {
        switch (r) {
            case Role::System: return "system";
            case Role::User: return "user";
            case Role::Assistant: return "assistant";
            default: return "user";
        }
    }
};

/**
 * @brief Result of an LLM API request
 */
struct LLMResponse
{
    bool success = false;
    std::string content;
    std::string error;
    int tokensUsed = 0;
};

/**
 * @brief Client for communicating with OpenAI-compatible LLM APIs
 * 
 * Provides synchronous and asynchronous chat completion requests.
 * The client manages conversation history and system prompts.
 * Uses WinHTTP on Windows for HTTP communication.
 */
class LLMClient
{
public:
    using ResponseCallback = std::function<void(const LLMResponse&)>;
    
    LLMClient();
    ~LLMClient();
    
    /**
     * @brief Configure the LLM endpoint and parameters
     * @param config LLM configuration
     */
    void configure(const LLMConfig& config);
    
    /**
     * @brief Get the current configuration
     * @return Current LLM configuration
     */
    const LLMConfig& getConfig() const { return m_config; }
    
    /**
     * @brief Set the system prompt that provides project context
     * @param systemPrompt The system prompt text
     */
    void setSystemPrompt(const std::string& systemPrompt);
    
    /**
     * @brief Get the current system prompt
     * @return Current system prompt
     */
    const std::string& getSystemPrompt() const { return m_systemPrompt; }
    
    /**
     * @brief Send a chat message and get a response (synchronous)
     * @param userMessage The user's message
     * @return LLM response
     */
    LLMResponse chat(const std::string& userMessage);
    
    /**
     * @brief Send a chat message and get a response (asynchronous)
     * @param userMessage The user's message
     * @param callback Function called with the response
     */
    void chatAsync(const std::string& userMessage, ResponseCallback callback);
    
    /**
     * @brief Get the conversation history
     * @return Vector of messages in the conversation
     */
    const std::vector<LLMMessage>& getHistory() const { return m_history; }
    
    /**
     * @brief Clear conversation history (keeps system prompt)
     */
    void clearHistory();
    
    /**
     * @brief Build the JSON request body for the API
     * @param messages Messages to include in the request
     * @return JSON string for the request body
     */
    std::string buildRequestBody(const std::vector<LLMMessage>& messages) const;
    
    /**
     * @brief Parse the JSON response from the API
     * @param responseBody Raw JSON response body
     * @return Parsed LLM response
     */
    static LLMResponse parseResponse(const std::string& responseBody);
    
    /**
     * @brief Check if the client is currently processing a request
     * @return true if a request is in progress
     */
    bool isProcessing() const { return m_processing; }
    
    /**
     * @brief Check if the client is configured and ready to use
     * @return true if configured
     */
    bool isConfigured() const { return !m_config.endpoint.empty(); }

private:
    /**
     * @brief Send an HTTP POST request (platform-specific implementation)
     * @param url The URL to send the request to
     * @param body The request body
     * @param apiKey The API key for authentication
     * @param timeoutSeconds Request timeout in seconds
     * @return Response body string, or empty on error
     */
    static std::string httpPost(const std::string& url, const std::string& body,
                                const std::string& apiKey, int timeoutSeconds);
    
    LLMConfig m_config;
    std::string m_systemPrompt;
    std::vector<LLMMessage> m_history;
    bool m_processing = false;
    std::mutex m_mutex;
};

} // namespace fresh
