#pragma once

#include <chrono>
#include <fstream>
#include <iomanip>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace fresh
{

/**
 * @brief Log severity levels
 */
enum class LogLevel {
    INFO,
    WARNING,
    ERR, // Renamed from ERROR to avoid Windows macro conflict
    FATAL
};

/**
 * @brief Interface for log listeners (e.g., UI console panels)
 */
class ILogListener
{
public:
    virtual ~ILogListener() = default;
    virtual void onLogMessage(LogLevel level, const std::string& message, const std::string& component) = 0;
};

/**
 * @brief Thread-safe logger class for outputting logs to files
 *
 * Logs are written to:
 * - logs/application_errors.txt for general application logs
 * - logs/Environment/{Platform}/ for platform-specific error logs
 */
class Logger
{
public:
    /**
     * @brief Get the singleton instance of the Logger
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger with optional custom log directory
     * @param logDir Base directory for logs (default: "logs")
     * @return true if initialization was successful
     */
    bool initialize(const std::string& logDir = "logs");

    /**
     * @brief Shutdown the logger and close all file handles
     */
    void shutdown();

    /**
     * @brief Log a message at the specified level
     * @param level Log severity level
     * @param message Message to log
     * @param component Optional component/subsystem name
     */
    void log(LogLevel level, const std::string& message, const std::string& component = "");

    /**
     * @brief Log an info message
     */
    void info(const std::string& message, const std::string& component = "");

    /**
     * @brief Log a warning message
     */
    void warning(const std::string& message, const std::string& component = "");

    /**
     * @brief Log an error message
     */
    void error(const std::string& message, const std::string& component = "");

    /**
     * @brief Log a fatal error message
     */
    void fatal(const std::string& message, const std::string& component = "");

    /**
     * @brief Enable/disable console output (in addition to file logging)
     */
    void setConsoleOutput(bool enabled)
    {
        m_consoleOutput = enabled;
    }

    /**
     * @brief Check if logger is initialized
     */
    [[nodiscard]] bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Add a log listener to receive log messages
     * @param listener Pointer to listener (must remain valid until removed)
     */
    void addListener(ILogListener* listener);

    /**
     * @brief Remove a log listener
     * @param listener Pointer to listener to remove
     */
    void removeListener(ILogListener* listener);

private:
    Logger();
    ~Logger();

    // Prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string getTimestamp() const;
    std::string getLevelString(LogLevel level) const;
    std::string getPlatformName() const;
    std::string getEnvironmentLogPath() const;
    void ensureDirectoryExists(const std::string& path);
    void writeToFile(std::ofstream& file, const std::string& message);

private:
    bool m_initialized;
    bool m_consoleOutput;
    std::string m_logDirectory;
    std::string m_applicationLogPath;
    std::string m_environmentLogPath;
    std::ofstream m_applicationLog;
    std::ofstream m_environmentLog;
    std::mutex m_mutex;
    std::vector<ILogListener*> m_listeners;
};

// Convenience macros for logging
#define LOG_INFO(msg)    fresh::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) fresh::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg)   fresh::Logger::getInstance().error(msg)
#define LOG_FATAL(msg)   fresh::Logger::getInstance().fatal(msg)

#define LOG_INFO_C(msg, component)    fresh::Logger::getInstance().info(msg, component)
#define LOG_WARNING_C(msg, component) fresh::Logger::getInstance().warning(msg, component)
#define LOG_ERROR_C(msg, component)   fresh::Logger::getInstance().error(msg, component)
#define LOG_FATAL_C(msg, component)   fresh::Logger::getInstance().fatal(msg, component)

} // namespace fresh
