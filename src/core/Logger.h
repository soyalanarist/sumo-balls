#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace logging {

/// Log severity levels
enum class Level : int {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Critical = 4
};

/// Convert log level to string
inline const char* levelToString(Level level) {
    switch (level) {
        case Level::Debug:    return "DEBUG";
        case Level::Info:     return "INFO";
        case Level::Warning:  return "WARN";
        case Level::Error:    return "ERROR";
        case Level::Critical: return "CRITICAL";
        default:              return "UNKNOWN";
    }
}

/// Get color code for terminal output (ANSI escape codes)
inline const char* levelToColor(Level level) {
    switch (level) {
        case Level::Debug:    return "\033[36m";      // Cyan
        case Level::Info:     return "\033[32m";      // Green
        case Level::Warning:  return "\033[33m";      // Yellow
        case Level::Error:    return "\033[31m";      // Red
        case Level::Critical: return "\033[1;31m";    // Bold Red
        default:              return "\033[0m";       // Reset
    }
}

inline const char* getColorReset() { return "\033[0m"; }

/// Global logger singleton
class Logger {
public:
    /// Get the global logger instance
    static Logger& instance();
    
    /// Initialize logger with file output
    /// @param filePath Path to log file, empty string for console-only
    /// @param level Minimum log level to output
    /// @param useColors Whether to use ANSI colors in console output
    void initialize(const std::string& filePath = "", Level level = Level::Info, bool useColors = true);
    
    /// Set minimum log level
    void setLevel(Level level) { minLevel = level; }
    
    /// Check if a log level should be output
    bool shouldLog(Level level) const { return static_cast<int>(level) >= static_cast<int>(minLevel); }
    
    /// Log a message
    /// @param level Log severity level
    /// @param module Module/component name
    /// @param message The log message
    void log(Level level, const std::string& module, const std::string& message);
    
    /// Convenience methods
    void debug(const std::string& module, const std::string& message) {
        if (shouldLog(Level::Debug)) log(Level::Debug, module, message);
    }
    
    void info(const std::string& module, const std::string& message) {
        if (shouldLog(Level::Info)) log(Level::Info, module, message);
    }
    
    void warning(const std::string& module, const std::string& message) {
        if (shouldLog(Level::Warning)) log(Level::Warning, module, message);
    }
    
    void error(const std::string& module, const std::string& message) {
        if (shouldLog(Level::Error)) log(Level::Error, module, message);
    }
    
    void critical(const std::string& module, const std::string& message) {
        if (shouldLog(Level::Critical)) log(Level::Critical, module, message);
    }
    
    /// Flush all output
    void flush();
    
    /// Close log file
    void shutdown();
    
    ~Logger() { shutdown(); }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::mutex logMutex;
    std::ofstream logFile;
    Level minLevel = Level::Info;
    bool useColors = true;
    bool initialized = false;
    
    /// Format a log message with timestamp
    std::string formatMessage(Level level, const std::string& module, const std::string& message);
};

// Convenience macros for logging
#define LOG_DEBUG(module, msg)    ::logging::Logger::instance().debug(module, msg)
#define LOG_INFO(module, msg)     ::logging::Logger::instance().info(module, msg)
#define LOG_WARN(module, msg)     ::logging::Logger::instance().warning(module, msg)
#define LOG_ERROR(module, msg)    ::logging::Logger::instance().error(module, msg)
#define LOG_CRITICAL(module, msg) ::logging::Logger::instance().critical(module, msg)

} // namespace logging
