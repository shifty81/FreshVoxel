#pragma once

#ifdef _WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <richedit.h>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "ui/native/Win32Panel.h"
#include "ai/LLMClient.h"
#include "ai/ProjectContext.h"

namespace fresh
{

/**
 * @brief Native Win32 LLM Assistant Panel for AI-powered game development help
 * 
 * Provides a chat interface integrated into the editor that connects to an
 * OpenAI-compatible LLM API (local or remote). The assistant has deep knowledge
 * of the Fresh Voxel Engine and can help with:
 * - Lua scripting for game logic
 * - Voxel world building and terrain design
 * - Editor tool usage and workflows
 * - NPC creation and AI behavior configuration
 * - Game design patterns and best practices
 * 
 * Supports local LLMs (Ollama, LM Studio) and cloud APIs (OpenAI).
 * Uses Unreal Engine-style dark theme consistent with other editor panels.
 */
class Win32LLMAssistantPanel : public Win32Panel
{
public:
    Win32LLMAssistantPanel();
    virtual ~Win32LLMAssistantPanel();

    /**
     * @brief Create the assistant panel
     * @param parent Parent window handle
     * @param x X position
     * @param y Y position
     * @param width Panel width
     * @param height Panel height
     * @return true if successful
     */
    bool create(HWND parent, int x, int y, int width, int height);

    /**
     * @brief Send a message to the LLM and display the response
     * @param message The user's message
     */
    void sendMessage(const std::string& message);

    /**
     * @brief Clear the chat history
     */
    void clearChat();

    /**
     * @brief Configure the LLM endpoint
     * @param config LLM configuration
     */
    void configure(const LLMConfig& config);

    /**
     * @brief Set the context topic for focused assistance
     * @param topic The topic to focus on
     */
    void setTopic(ProjectContext::Topic topic);

    /**
     * @brief Get the LLM client for external configuration
     * @return Pointer to the LLM client
     */
    LLMClient* getLLMClient() { return &m_llmClient; }

    /**
     * @brief Check if the assistant is currently processing a request
     * @return true if processing
     */
    bool isProcessing() const { return m_llmClient.isProcessing(); }

protected:
    void onCreate() override;
    void onResize(int width, int height) override;
    void onDestroy() override;
    bool handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result) override;

private:
    void createControls();
    void layoutControls();
    void appendChatMessage(const std::string& sender, const std::string& message, bool isUser);
    void appendToRichEdit(const std::string& text, COLORREF color, bool bold = false);
    void handleSend();
    void onResponseReceived(const LLMResponse& response);
    void updateTopicCombo();

    // Child controls
    HWND m_chatDisplay;       // RichEdit control for chat history
    HWND m_inputEdit;         // Edit control for message input
    HWND m_sendButton;        // Send button
    HWND m_clearButton;       // Clear chat button
    HWND m_topicCombo;        // Topic selector combobox
    HWND m_statusLabel;       // Status text label
    HWND m_settingsButton;    // Settings/configure button

    // State
    LLMClient m_llmClient;
    ProjectContext m_projectContext;
    ProjectContext::Topic m_currentTopic;

    // Custom message for async response delivery
    static constexpr UINT WM_LLM_RESPONSE = WM_USER + 100;

    // Layout constants
    static constexpr int TOOLBAR_HEIGHT = 30;
    static constexpr int INPUT_HEIGHT = 60;
    static constexpr int BUTTON_WIDTH = 60;
    static constexpr int MARGIN = 5;
    static constexpr int COMBO_WIDTH = 130;
    static constexpr int STATUS_HEIGHT = 20;
};

} // namespace fresh

#endif // _WIN32
