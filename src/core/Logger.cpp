#include "Logger.h"

namespace logging {

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::initialize(const std::string& filePath, Level level, bool colors) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    minLevel = level;
    useColors = colors;
    
    if (!filePath.empty()) {
        logFile.open(filePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "[Logger] Failed to open log file: " << filePath << std::endl;
        }
    }
    
    initialized = true;
}

std::string Logger::formatMessage(Level level, const std::string& module, const std::string& message) {
    std::ostringstream oss;
    
    // Get current time with milliseconds
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::tm* timeinfo = std::localtime(&time_t);
    
    oss << std::put_time(timeinfo, "%H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count()
        << " [" << std::setfill(' ') << std::setw(8) << levelToString(level) << "] "
        << "[" << std::setw(12) << module << "] "
        << message;
    
    return oss.str();
}

void Logger::log(Level level, const std::string& module, const std::string& message) {
    if (!shouldLog(level)) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string formatted = formatMessage(level, module, message);
    
    // Output to console with colors if enabled
    if (useColors) {
        std::cerr << levelToColor(level) << formatted << getColorReset() << std::endl;
    } else {
        std::cerr << formatted << std::endl;
    }
    
    // Output to file if open
    if (logFile.is_open()) {
        logFile << formatted << std::endl;
        logFile.flush();
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.flush();
    }
}

void Logger::shutdown() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

} // namespace logging
