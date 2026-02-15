#include "ai/LLMClient.h"
#include "core/Logger.h"

#include <sstream>
#include <thread>

#ifdef _WIN32
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #include <windows.h>
    #include <winhttp.h>
    #pragma comment(lib, "winhttp.lib")
#endif

namespace fresh
{

// Simple JSON string escaping (no external dependency needed for basic use)
static std::string escapeJson(const std::string& input)
{
    std::string output;
    output.reserve(input.size() + 16);
    for (char c : input) {
        switch (c) {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n";  break;
            case '\r': output += "\\r";  break;
            case '\t': output += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    output += buf;
                } else {
                    output += c;
                }
                break;
        }
    }
    return output;
}

// Simple JSON string extraction helper
static std::string extractJsonString(const std::string& json, const std::string& key)
{
    std::string searchKey = "\"" + key + "\"";
    auto pos = json.find(searchKey);
    if (pos == std::string::npos) return "";
    
    // Find the colon after the key
    pos = json.find(':', pos + searchKey.size());
    if (pos == std::string::npos) return "";
    
    // Skip whitespace
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r')) {
        pos++;
    }
    
    if (pos >= json.size() || json[pos] != '"') return "";
    pos++; // skip opening quote
    
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            pos++;
            switch (json[pos]) {
                case '"':  result += '"'; break;
                case '\\': result += '\\'; break;
                case 'n':  result += '\n'; break;
                case 'r':  result += '\r'; break;
                case 't':  result += '\t'; break;
                default:   result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        pos++;
    }
    return result;
}

LLMClient::LLMClient() = default;

LLMClient::~LLMClient() = default;

void LLMClient::configure(const LLMConfig& config)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
    LOG_INFO_C("LLM client configured - endpoint: " + config.endpoint + 
               ", model: " + config.model, "LLMClient");
}

void LLMClient::setSystemPrompt(const std::string& systemPrompt)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_systemPrompt = systemPrompt;
}

LLMResponse LLMClient::chat(const std::string& userMessage)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isConfigured()) {
        LLMResponse response;
        response.success = false;
        response.error = "LLM client not configured. Please set an API endpoint in the assistant settings.";
        return response;
    }
    
    m_processing = true;
    
    // Add user message to history
    m_history.push_back({LLMMessage::Role::User, userMessage});
    
    // Build messages list: system prompt + conversation history
    std::vector<LLMMessage> messages;
    if (!m_systemPrompt.empty()) {
        messages.push_back({LLMMessage::Role::System, m_systemPrompt});
    }
    
    // Include recent history (limit to avoid token overflow)
    const size_t maxHistoryMessages = 20;
    size_t startIdx = 0;
    if (m_history.size() > maxHistoryMessages) {
        startIdx = m_history.size() - maxHistoryMessages;
    }
    for (size_t i = startIdx; i < m_history.size(); i++) {
        messages.push_back(m_history[i]);
    }
    
    // Build request
    std::string requestBody = buildRequestBody(messages);
    
    // Send HTTP request
    std::string responseBody = httpPost(m_config.endpoint, requestBody, 
                                        m_config.apiKey, m_config.timeoutSeconds);
    
    // Parse response
    LLMResponse response;
    if (responseBody.empty()) {
        response.success = false;
        response.error = "No response from LLM endpoint. Check that the server is running at: " + m_config.endpoint;
    } else {
        response = parseResponse(responseBody);
    }
    
    // Add assistant response to history
    if (response.success) {
        m_history.push_back({LLMMessage::Role::Assistant, response.content});
    }
    
    m_processing = false;
    return response;
}

void LLMClient::chatAsync(const std::string& userMessage, ResponseCallback callback)
{
    std::thread([this, userMessage, callback]() {
        LLMResponse response = chat(userMessage);
        if (callback) {
            callback(response);
        }
    }).detach();
}

void LLMClient::clearHistory()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_history.clear();
    LOG_INFO_C("Conversation history cleared", "LLMClient");
}

std::string LLMClient::buildRequestBody(const std::vector<LLMMessage>& messages) const
{
    std::ostringstream json;
    json << "{\"model\":\"" << escapeJson(m_config.model) << "\",";
    json << "\"messages\":[";
    
    for (size_t i = 0; i < messages.size(); i++) {
        if (i > 0) json << ",";
        json << "{\"role\":\"" << LLMMessage::roleToString(messages[i].role) << "\",";
        json << "\"content\":\"" << escapeJson(messages[i].content) << "\"}";
    }
    
    json << "],";
    json << "\"temperature\":" << m_config.temperature << ",";
    json << "\"max_tokens\":" << m_config.maxTokens;
    json << "}";
    
    return json.str();
}

LLMResponse LLMClient::parseResponse(const std::string& responseBody)
{
    LLMResponse response;
    
    // Check for error response
    std::string errorMsg = extractJsonString(responseBody, "message");
    if (!errorMsg.empty() && responseBody.find("\"error\"") != std::string::npos) {
        response.success = false;
        response.error = errorMsg;
        return response;
    }
    
    // Extract content from choices[0].message.content
    // Look for "content" field in the response
    std::string content = extractJsonString(responseBody, "content");
    if (!content.empty()) {
        response.success = true;
        response.content = content;
    } else {
        response.success = false;
        response.error = "Failed to parse LLM response";
    }
    
    return response;
}

#ifdef _WIN32

// Parse URL into components for WinHTTP
struct ParsedUrl {
    std::wstring host;
    std::wstring path;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTP_PORT;
    bool useSSL = false;
};

static ParsedUrl parseUrl(const std::string& url)
{
    ParsedUrl result;
    
    std::string remaining = url;
    
    // Check for HTTPS
    if (remaining.substr(0, 8) == "https://") {
        result.useSSL = true;
        result.port = INTERNET_DEFAULT_HTTPS_PORT;
        remaining = remaining.substr(8);
    } else if (remaining.substr(0, 7) == "http://") {
        remaining = remaining.substr(7);
    }
    
    // Find path
    auto pathPos = remaining.find('/');
    std::string hostPort;
    std::string path;
    if (pathPos != std::string::npos) {
        hostPort = remaining.substr(0, pathPos);
        path = remaining.substr(pathPos);
    } else {
        hostPort = remaining;
        path = "/";
    }
    
    // Parse host and port
    auto colonPos = hostPort.find(':');
    std::string host;
    if (colonPos != std::string::npos) {
        host = hostPort.substr(0, colonPos);
        std::string portStr = hostPort.substr(colonPos + 1);
        int portNum = std::stoi(portStr);
        if (portNum > 0 && portNum <= 65535) {
            result.port = static_cast<INTERNET_PORT>(portNum);
        }
    } else {
        host = hostPort;
    }
    
    // Convert to wide strings
    result.host = std::wstring(host.begin(), host.end());
    result.path = std::wstring(path.begin(), path.end());
    
    return result;
}

std::string LLMClient::httpPost(const std::string& url, const std::string& body,
                                 const std::string& apiKey, int timeoutSeconds)
{
    ParsedUrl parsed = parseUrl(url);
    
    HINTERNET hSession = WinHttpOpen(L"FreshVoxelEngine/1.0",
                                      WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                      WINHTTP_NO_PROXY_NAME,
                                      WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        LOG_ERROR_C("WinHTTP: Failed to open session", "LLMClient");
        return "";
    }
    
    // Set timeouts
    DWORD timeout = static_cast<DWORD>(timeoutSeconds * 1000);
    WinHttpSetTimeouts(hSession, timeout, timeout, timeout, timeout);
    
    HINTERNET hConnect = WinHttpConnect(hSession, parsed.host.c_str(), parsed.port, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        LOG_ERROR_C("WinHTTP: Failed to connect to " + url, "LLMClient");
        return "";
    }
    
    DWORD flags = parsed.useSSL ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", parsed.path.c_str(),
                                             nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        LOG_ERROR_C("WinHTTP: Failed to open request", "LLMClient");
        return "";
    }
    
    // Set headers
    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!apiKey.empty()) {
        headers += L"Authorization: Bearer ";
        headers += std::wstring(apiKey.begin(), apiKey.end());
        headers += L"\r\n";
    }
    
    WinHttpAddRequestHeaders(hRequest, headers.c_str(), static_cast<DWORD>(headers.size()),
                              WINHTTP_ADDREQ_FLAG_ADD);
    
    // Send request
    BOOL result = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      (LPVOID)body.c_str(), static_cast<DWORD>(body.size()),
                                      static_cast<DWORD>(body.size()), 0);
    
    std::string responseBody;
    
    if (result && WinHttpReceiveResponse(hRequest, nullptr)) {
        DWORD bytesAvailable = 0;
        DWORD bytesRead = 0;
        
        do {
            bytesAvailable = 0;
            WinHttpQueryDataAvailable(hRequest, &bytesAvailable);
            
            if (bytesAvailable > 0) {
                std::vector<char> buffer(bytesAvailable + 1, 0);
                WinHttpReadData(hRequest, buffer.data(), bytesAvailable, &bytesRead);
                responseBody.append(buffer.data(), bytesRead);
            }
        } while (bytesAvailable > 0);
    } else {
        DWORD error = GetLastError();
        LOG_ERROR_C("WinHTTP: Request failed with error " + std::to_string(error), "LLMClient");
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return responseBody;
}

#else
// Stub implementation for non-Windows platforms (CI/testing)
std::string LLMClient::httpPost(const std::string& url, const std::string& body,
                                 const std::string& apiKey, int timeoutSeconds)
{
    (void)url;
    (void)body;
    (void)apiKey;
    (void)timeoutSeconds;
    LOG_WARNING_C("HTTP POST not available on this platform", "LLMClient");
    return "";
}
#endif // _WIN32

} // namespace fresh
