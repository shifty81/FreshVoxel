#ifdef _WIN32

#include "ui/native/Win32LLMAssistantPanel.h"
#include "ui/native/UnrealStyleTheme.h"
#include "core/Logger.h"
#include <commctrl.h>
#include <sstream>

namespace fresh
{

// Control IDs
static constexpr int IDC_CHAT_DISPLAY = 3001;
static constexpr int IDC_INPUT_EDIT = 3002;
static constexpr int IDC_SEND_BUTTON = 3003;
static constexpr int IDC_CLEAR_BUTTON = 3004;
static constexpr int IDC_TOPIC_COMBO = 3005;
static constexpr int IDC_SETTINGS_BUTTON = 3006;

Win32LLMAssistantPanel::Win32LLMAssistantPanel()
    : Win32Panel(),
      m_chatDisplay(nullptr),
      m_inputEdit(nullptr),
      m_sendButton(nullptr),
      m_clearButton(nullptr),
      m_topicCombo(nullptr),
      m_statusLabel(nullptr),
      m_settingsButton(nullptr),
      m_currentTopic(ProjectContext::Topic::General)
{
}

Win32LLMAssistantPanel::~Win32LLMAssistantPanel() = default;

bool Win32LLMAssistantPanel::create(HWND parent, int x, int y, int width, int height)
{
    // Load RichEdit library
    static bool richEditLoaded = false;
    if (!richEditLoaded) {
        LoadLibrary(TEXT("Riched20.dll"));
        richEditLoaded = true;
    }

    if (!Win32Panel::create(parent, x, y, width, height, L"AI Assistant")) {
        return false;
    }

    createControls();
    
    // Set default configuration for local LLM (Ollama)
    LLMConfig defaultConfig;
    defaultConfig.endpoint = "http://localhost:11434/v1/chat/completions";
    defaultConfig.model = "llama3";
    m_llmClient.configure(defaultConfig);
    
    // Set initial project context
    std::string systemPrompt = m_projectContext.generateSystemPrompt(m_currentTopic);
    m_llmClient.setSystemPrompt(systemPrompt);

    // Show welcome message
    appendToRichEdit("Fresh Voxel Engine AI Assistant\r\n", UnrealStyleTheme::AccentBlue, true);
    appendToRichEdit("================================\r\n", UnrealStyleTheme::TextSecondary);
    appendToRichEdit("Ask me anything about building your game!\r\n", UnrealStyleTheme::TextSecondary);
    appendToRichEdit("Topics: Lua scripting, voxel building, editor tools, NPC/AI, game design\r\n\r\n", 
                     UnrealStyleTheme::TextSecondary);
    appendToRichEdit("Configure your LLM endpoint via the Settings button.\r\n", 
                     UnrealStyleTheme::TextSecondary);
    appendToRichEdit("Default: Ollama at localhost:11434\r\n\r\n", UnrealStyleTheme::TextSecondary);

    LOG_INFO_C("LLM Assistant Panel created", "Win32LLMAssistantPanel");
    return true;
}

void Win32LLMAssistantPanel::onCreate()
{
    createControls();
}

void Win32LLMAssistantPanel::createControls()
{
    if (!m_hwnd) return;

    int y = getContentYOffset() + MARGIN;
    int x = MARGIN;
    int controlWidth = m_width - 2 * MARGIN;

    // Toolbar row: Topic selector + Settings + Clear buttons
    m_topicCombo = CreateWindowExW(
        0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        x, y, COMBO_WIDTH, 200,
        m_hwnd, (HMENU)(INT_PTR)IDC_TOPIC_COMBO, nullptr, nullptr);
    
    if (m_topicCombo) {
        SendMessageW(m_topicCombo, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
        updateTopicCombo();
    }

    m_settingsButton = CreateWindowExW(
        0, L"BUTTON", L"Settings",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + COMBO_WIDTH + MARGIN, y, BUTTON_WIDTH + 10, TOOLBAR_HEIGHT - 4,
        m_hwnd, (HMENU)(INT_PTR)IDC_SETTINGS_BUTTON, nullptr, nullptr);
    if (m_settingsButton) {
        SendMessageW(m_settingsButton, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    }

    m_clearButton = CreateWindowExW(
        0, L"BUTTON", L"Clear",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x + COMBO_WIDTH + BUTTON_WIDTH + MARGIN * 2 + 10, y, BUTTON_WIDTH, TOOLBAR_HEIGHT - 4,
        m_hwnd, (HMENU)(INT_PTR)IDC_CLEAR_BUTTON, nullptr, nullptr);
    if (m_clearButton) {
        SendMessageW(m_clearButton, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    }

    y += TOOLBAR_HEIGHT;

    // Chat display (RichEdit) - takes most of the panel
    int chatHeight = m_height - getContentYOffset() - TOOLBAR_HEIGHT - INPUT_HEIGHT - STATUS_HEIGHT - MARGIN * 5;
    m_chatDisplay = CreateWindowExW(
        0, MSFTEDIT_CLASS, L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        x, y, controlWidth, chatHeight,
        m_hwnd, (HMENU)(INT_PTR)IDC_CHAT_DISPLAY, nullptr, nullptr);
    
    if (m_chatDisplay) {
        // Set dark theme colors
        SendMessageW(m_chatDisplay, EM_SETBKGNDCOLOR, 0, (LPARAM)UnrealStyleTheme::DarkBackground);
        
        // Set font
        CHARFORMATW cf = {};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_FACE | CFM_SIZE | CFM_COLOR;
        cf.yHeight = 180; // 9pt in twips
        cf.crTextColor = UnrealStyleTheme::TextPrimary;
        wcscpy_s(cf.szFaceName, L"Consolas");
        SendMessageW(m_chatDisplay, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    }

    y += chatHeight + MARGIN;

    // Input area - multiline edit with send button
    int inputWidth = controlWidth - BUTTON_WIDTH - MARGIN;
    m_inputEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        x, y, inputWidth, INPUT_HEIGHT,
        m_hwnd, (HMENU)(INT_PTR)IDC_INPUT_EDIT, nullptr, nullptr);
    
    if (m_inputEdit) {
        SendMessageW(m_inputEdit, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetMonospaceFont(), TRUE);
        // Set placeholder text
        SendMessageW(m_inputEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"Ask about your game...");
    }

    m_sendButton = CreateWindowExW(
        0, L"BUTTON", L"Send",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_DEFPUSHBUTTON,
        x + inputWidth + MARGIN, y, BUTTON_WIDTH, INPUT_HEIGHT,
        m_hwnd, (HMENU)(INT_PTR)IDC_SEND_BUTTON, nullptr, nullptr);
    if (m_sendButton) {
        SendMessageW(m_sendButton, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetBoldFont(), TRUE);
    }

    y += INPUT_HEIGHT + MARGIN;

    // Status label at bottom
    m_statusLabel = CreateWindowExW(
        0, L"STATIC", L"Ready - Configure LLM endpoint in Settings",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, controlWidth, STATUS_HEIGHT,
        m_hwnd, nullptr, nullptr, nullptr);
    if (m_statusLabel) {
        SendMessageW(m_statusLabel, WM_SETFONT, (WPARAM)UnrealStyleTheme::GetFont(), TRUE);
    }
}

void Win32LLMAssistantPanel::onResize(int width, int height)
{
    (void)width;
    (void)height;
    layoutControls();
}

void Win32LLMAssistantPanel::layoutControls()
{
    if (!m_hwnd) return;

    int y = getContentYOffset() + MARGIN;
    int x = MARGIN;
    int controlWidth = m_width - 2 * MARGIN;

    // Toolbar
    if (m_topicCombo) {
        SetWindowPos(m_topicCombo, nullptr, x, y, COMBO_WIDTH, 200, SWP_NOZORDER);
    }
    if (m_settingsButton) {
        SetWindowPos(m_settingsButton, nullptr, x + COMBO_WIDTH + MARGIN, y, 
                     BUTTON_WIDTH + 10, TOOLBAR_HEIGHT - 4, SWP_NOZORDER);
    }
    if (m_clearButton) {
        SetWindowPos(m_clearButton, nullptr, x + COMBO_WIDTH + BUTTON_WIDTH + MARGIN * 2 + 10, y,
                     BUTTON_WIDTH, TOOLBAR_HEIGHT - 4, SWP_NOZORDER);
    }

    y += TOOLBAR_HEIGHT;

    // Chat display
    int chatHeight = m_height - getContentYOffset() - TOOLBAR_HEIGHT - INPUT_HEIGHT - STATUS_HEIGHT - MARGIN * 5;
    if (chatHeight < 50) chatHeight = 50;
    if (m_chatDisplay) {
        SetWindowPos(m_chatDisplay, nullptr, x, y, controlWidth, chatHeight, SWP_NOZORDER);
    }

    y += chatHeight + MARGIN;

    // Input area
    int inputWidth = controlWidth - BUTTON_WIDTH - MARGIN;
    if (m_inputEdit) {
        SetWindowPos(m_inputEdit, nullptr, x, y, inputWidth, INPUT_HEIGHT, SWP_NOZORDER);
    }
    if (m_sendButton) {
        SetWindowPos(m_sendButton, nullptr, x + inputWidth + MARGIN, y, 
                     BUTTON_WIDTH, INPUT_HEIGHT, SWP_NOZORDER);
    }

    y += INPUT_HEIGHT + MARGIN;

    // Status label
    if (m_statusLabel) {
        SetWindowPos(m_statusLabel, nullptr, x, y, controlWidth, STATUS_HEIGHT, SWP_NOZORDER);
    }
}

void Win32LLMAssistantPanel::onDestroy()
{
    // Controls are child windows, destroyed automatically
}

bool Win32LLMAssistantPanel::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)
{
    switch (msg) {
        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            int notifyCode = HIWORD(wParam);
            
            if (controlId == IDC_SEND_BUTTON && notifyCode == BN_CLICKED) {
                handleSend();
                result = 0;
                return true;
            }
            
            if (controlId == IDC_CLEAR_BUTTON && notifyCode == BN_CLICKED) {
                clearChat();
                result = 0;
                return true;
            }
            
            if (controlId == IDC_SETTINGS_BUTTON && notifyCode == BN_CLICKED) {
                // Show a simple settings dialog
                // For now, prompt for endpoint URL
                wchar_t buffer[512] = {};
                std::string currentEndpoint = m_llmClient.getConfig().endpoint;
                std::wstring wEndpoint(currentEndpoint.begin(), currentEndpoint.end());
                wcscpy_s(buffer, wEndpoint.c_str());
                
                // Use a simple input approach - log instructions to chat
                appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
                appendToRichEdit("To configure, type a command:\r\n", UnrealStyleTheme::TextSecondary);
                appendToRichEdit("  /endpoint <url>  - Set API endpoint\r\n", UnrealStyleTheme::TextSecondary);
                appendToRichEdit("  /model <name>    - Set model name\r\n", UnrealStyleTheme::TextSecondary);
                appendToRichEdit("  /key <api-key>   - Set API key\r\n", UnrealStyleTheme::TextSecondary);
                appendToRichEdit("  /status          - Show current config\r\n\r\n", UnrealStyleTheme::TextSecondary);
                
                result = 0;
                return true;
            }
            
            if (controlId == IDC_TOPIC_COMBO && notifyCode == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(m_topicCombo, CB_GETCURSEL, 0, 0);
                if (sel >= 0 && sel <= static_cast<int>(ProjectContext::Topic::GameDesign)) {
                    m_currentTopic = static_cast<ProjectContext::Topic>(sel);
                    std::string systemPrompt = m_projectContext.generateSystemPrompt(m_currentTopic);
                    m_llmClient.setSystemPrompt(systemPrompt);
                    
                    appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
                    appendToRichEdit("Topic changed to: " + ProjectContext::topicToString(m_currentTopic) + "\r\n\r\n",
                                     UnrealStyleTheme::TextSecondary);
                }
                result = 0;
                return true;
            }
            break;
        }
        
        case WM_LLM_RESPONSE: {
            // Async response received - the LLMResponse is passed via lParam
            LLMResponse* responsePtr = reinterpret_cast<LLMResponse*>(lParam);
            if (responsePtr) {
                onResponseReceived(*responsePtr);
                delete responsePtr;
            }
            result = 0;
            return true;
        }
        
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
            SetBkColor(hdc, UnrealStyleTheme::InputBackground);
            result = (LRESULT)UnrealStyleTheme::GetInputBackgroundBrush();
            return true;
        }
        
        case WM_CTLCOLORLISTBOX: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, UnrealStyleTheme::TextPrimary);
            SetBkColor(hdc, UnrealStyleTheme::InputBackground);
            result = (LRESULT)UnrealStyleTheme::GetInputBackgroundBrush();
            return true;
        }
    }
    
    return Win32Panel::handleMessage(msg, wParam, lParam, result);
}

void Win32LLMAssistantPanel::sendMessage(const std::string& message)
{
    if (message.empty()) return;
    
    // Handle slash commands
    if (message[0] == '/') {
        std::istringstream iss(message);
        std::string command;
        iss >> command;
        
        if (command == "/endpoint") {
            std::string url;
            iss >> url;
            if (!url.empty()) {
                LLMConfig config = m_llmClient.getConfig();
                config.endpoint = url;
                m_llmClient.configure(config);
                appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
                appendToRichEdit("Endpoint set to: " + url + "\r\n\r\n", UnrealStyleTheme::TextSecondary);
            }
            return;
        }
        
        if (command == "/model") {
            std::string model;
            iss >> model;
            if (!model.empty()) {
                LLMConfig config = m_llmClient.getConfig();
                config.model = model;
                m_llmClient.configure(config);
                appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
                appendToRichEdit("Model set to: " + model + "\r\n\r\n", UnrealStyleTheme::TextSecondary);
            }
            return;
        }
        
        if (command == "/key") {
            std::string key;
            iss >> key;
            if (!key.empty()) {
                LLMConfig config = m_llmClient.getConfig();
                config.apiKey = key;
                m_llmClient.configure(config);
                appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
                appendToRichEdit("API key updated\r\n\r\n", UnrealStyleTheme::TextSecondary);
            }
            return;
        }
        
        if (command == "/status") {
            const auto& config = m_llmClient.getConfig();
            appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusInfo, true);
            appendToRichEdit("Current Configuration:\r\n", UnrealStyleTheme::TextSecondary);
            appendToRichEdit("  Endpoint: " + config.endpoint + "\r\n", UnrealStyleTheme::TextPrimary);
            appendToRichEdit("  Model: " + config.model + "\r\n", UnrealStyleTheme::TextPrimary);
            appendToRichEdit("  API Key: " + (config.apiKey.empty() ? "(not set)" : "(set)") + "\r\n", 
                             UnrealStyleTheme::TextPrimary);
            appendToRichEdit("  Topic: " + ProjectContext::topicToString(m_currentTopic) + "\r\n\r\n",
                             UnrealStyleTheme::TextPrimary);
            return;
        }
        
        appendToRichEdit("\r\n[System] ", UnrealStyleTheme::StatusWarning, true);
        appendToRichEdit("Unknown command: " + command + "\r\n\r\n", UnrealStyleTheme::TextSecondary);
        return;
    }
    
    // Display user message
    appendChatMessage("You", message, true);
    
    // Update status
    if (m_statusLabel) {
        SetWindowTextW(m_statusLabel, L"Thinking...");
    }
    
    // Disable send button while processing
    if (m_sendButton) {
        EnableWindow(m_sendButton, FALSE);
    }
    
    // Send async request - post response back to this window
    HWND hwnd = m_hwnd;
    m_llmClient.chatAsync(message, [hwnd](const LLMResponse& response) {
        // Allocate response on heap to pass via Windows message
        LLMResponse* heapResponse = new LLMResponse(response);
        PostMessageW(hwnd, WM_LLM_RESPONSE, 0, reinterpret_cast<LPARAM>(heapResponse));
    });
}

void Win32LLMAssistantPanel::onResponseReceived(const LLMResponse& response)
{
    // Re-enable send button
    if (m_sendButton) {
        EnableWindow(m_sendButton, TRUE);
    }
    
    if (response.success) {
        appendChatMessage("Assistant", response.content, false);
        if (m_statusLabel) {
            SetWindowTextW(m_statusLabel, L"Ready");
        }
    } else {
        appendToRichEdit("\r\n[Error] ", UnrealStyleTheme::StatusError, true);
        appendToRichEdit(response.error + "\r\n\r\n", UnrealStyleTheme::TextSecondary);
        if (m_statusLabel) {
            std::wstring errorW(response.error.begin(), response.error.end());
            SetWindowTextW(m_statusLabel, (L"Error: " + errorW).c_str());
        }
        LOG_ERROR_C("LLM error: " + response.error, "Win32LLMAssistantPanel");
    }
}

void Win32LLMAssistantPanel::clearChat()
{
    m_llmClient.clearHistory();
    if (m_chatDisplay) {
        SetWindowTextW(m_chatDisplay, L"");
    }
    appendToRichEdit("Chat cleared. Ask me anything!\r\n\r\n", UnrealStyleTheme::TextSecondary);
    LOG_INFO_C("Chat history cleared", "Win32LLMAssistantPanel");
}

void Win32LLMAssistantPanel::configure(const LLMConfig& config)
{
    m_llmClient.configure(config);
}

void Win32LLMAssistantPanel::setTopic(ProjectContext::Topic topic)
{
    m_currentTopic = topic;
    std::string systemPrompt = m_projectContext.generateSystemPrompt(topic);
    m_llmClient.setSystemPrompt(systemPrompt);
    
    // Update combo box selection
    if (m_topicCombo) {
        SendMessageW(m_topicCombo, CB_SETCURSEL, static_cast<int>(topic), 0);
    }
}

void Win32LLMAssistantPanel::handleSend()
{
    if (!m_inputEdit) return;
    
    // Get text from input
    int len = GetWindowTextLengthW(m_inputEdit);
    if (len <= 0) return;
    
    std::vector<wchar_t> buffer(len + 1);
    GetWindowTextW(m_inputEdit, buffer.data(), len + 1);
    
    // Convert to narrow string
    std::string message;
    for (int i = 0; i < len; i++) {
        if (buffer[i] < 128) {
            message += static_cast<char>(buffer[i]);
        } else {
            // Basic UTF-8 encoding for non-ASCII characters
            char mbBuf[4];
            int mbLen = WideCharToMultiByte(CP_UTF8, 0, &buffer[i], 1, mbBuf, 4, nullptr, nullptr);
            message.append(mbBuf, mbLen);
        }
    }
    
    if (message.empty()) return;
    
    // Clear input
    SetWindowTextW(m_inputEdit, L"");
    
    // Send message
    sendMessage(message);
    
    // Focus back on input
    SetFocus(m_inputEdit);
}

void Win32LLMAssistantPanel::appendChatMessage(const std::string& sender, const std::string& message, bool isUser)
{
    COLORREF senderColor = isUser ? UnrealStyleTheme::AccentBlue : UnrealStyleTheme::StatusSuccess;
    COLORREF textColor = UnrealStyleTheme::TextPrimary;
    
    appendToRichEdit("\r\n" + sender + ":\r\n", senderColor, true);
    appendToRichEdit(message + "\r\n", textColor);
}

void Win32LLMAssistantPanel::appendToRichEdit(const std::string& text, COLORREF color, bool bold)
{
    if (!m_chatDisplay) return;
    
    // Move cursor to end
    int textLen = GetWindowTextLengthW(m_chatDisplay);
    SendMessageW(m_chatDisplay, EM_SETSEL, textLen, textLen);
    
    // Set character format
    CHARFORMATW cf = {};
    cf.cbSize = sizeof(cf);
    cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_FACE | CFM_SIZE;
    cf.crTextColor = color;
    cf.yHeight = 180; // 9pt
    cf.dwEffects = bold ? CFE_BOLD : 0;
    wcscpy_s(cf.szFaceName, L"Consolas");
    SendMessageW(m_chatDisplay, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    
    // Convert to wide string and insert
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), nullptr, 0);
    if (wideLen > 0) {
        std::vector<wchar_t> wideText(wideLen + 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), (int)text.size(), wideText.data(), wideLen);
        SendMessageW(m_chatDisplay, EM_REPLACESEL, FALSE, (LPARAM)wideText.data());
    }
    
    // Auto-scroll to bottom
    SendMessageW(m_chatDisplay, WM_VSCROLL, SB_BOTTOM, 0);
}

void Win32LLMAssistantPanel::updateTopicCombo()
{
    if (!m_topicCombo) return;
    
    SendMessageW(m_topicCombo, CB_RESETCONTENT, 0, 0);
    
    auto topics = ProjectContext::getTopicNames();
    for (const auto& topic : topics) {
        std::wstring wTopic(topic.begin(), topic.end());
        SendMessageW(m_topicCombo, CB_ADDSTRING, 0, (LPARAM)wTopic.c_str());
    }
    
    SendMessageW(m_topicCombo, CB_SETCURSEL, static_cast<int>(m_currentTopic), 0);
}

} // namespace fresh

#endif // _WIN32
