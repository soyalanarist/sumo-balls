#include "../TestFramework.h"
#include "../../src/core/Logger.h"
#include <fstream>
#include <cstdio>
#include <sstream>

// Alias updated logger namespace for backward compatibility in tests
namespace log = logging;

std::string readLogFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

TEST(Logger, LogsDebugMessage) {
    std::string logFile = "/tmp/test_debug.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Debug, true);
    log::Logger::instance().debug("TestModule", "This is a debug message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("DEBUG") != std::string::npos, "Should contain DEBUG level");
    TEST_ASSERT(content.find("TestModule") != std::string::npos, "Should contain module name");
    TEST_ASSERT(content.find("This is a debug message") != std::string::npos, "Should contain message");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, LogsInfoMessage) {
    std::string logFile = "/tmp/test_info.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Info, true);
    log::Logger::instance().info("TestModule", "Info message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("INFO") != std::string::npos, "Should contain INFO level");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, LogsWarningMessage) {
    std::string logFile = "/tmp/test_warn.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Warning, true);
    log::Logger::instance().warning("TestModule", "Warning message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("WARN") != std::string::npos, "Should contain WARN level");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, LogsErrorMessage) {
    std::string logFile = "/tmp/test_error.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Error, true);
    log::Logger::instance().error("TestModule", "Error message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("ERROR") != std::string::npos, "Should contain ERROR level");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, LogsCriticalMessage) {
    std::string logFile = "/tmp/test_critical.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Critical, true);
    log::Logger::instance().critical("TestModule", "Critical message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("CRITICAL") != std::string::npos, "Should contain CRITICAL level");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, IncludesTimestamp) {
    std::string logFile = "/tmp/test_timestamp.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Info, true);
    log::Logger::instance().info("TestModule", "Timestamped message");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find(":") != std::string::npos, "Should contain timestamp with colons");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, RespectLogLevel) {
    std::string logFile = "/tmp/test_level.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Warning, true);
    log::Logger::instance().debug("TestModule", "Should not appear");
    log::Logger::instance().info("TestModule", "Should not appear");
    log::Logger::instance().warning("TestModule", "Should appear");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("Should appear") != std::string::npos, "Warning should appear");
    TEST_ASSERT(content.find("Should not appear") == std::string::npos, "Debug/Info should not appear");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, ShouldLogCheck) {
    std::string logFile = "/tmp/test_should.log";
    log::Logger::instance().initialize(logFile, log::Level::Error, true);
    
    TEST_FALSE(log::Logger::instance().shouldLog(log::Level::Debug));
    TEST_FALSE(log::Logger::instance().shouldLog(log::Level::Info));
    TEST_FALSE(log::Logger::instance().shouldLog(log::Level::Warning));
    TEST_TRUE(log::Logger::instance().shouldLog(log::Level::Error));
    TEST_TRUE(log::Logger::instance().shouldLog(log::Level::Critical));
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, MultipleModules) {
    std::string logFile = "/tmp/test_modules.log";
    std::remove(logFile.c_str());
    
    log::Logger::instance().initialize(logFile, log::Level::Debug, true);
    log::Logger::instance().info("Module1", "From module 1");
    log::Logger::instance().info("Module2", "From module 2");
    log::Logger::instance().info("Module3", "From module 3");
    log::Logger::instance().flush();
    
    std::string content = readLogFile(logFile);
    TEST_ASSERT(content.find("Module1") != std::string::npos, "Should contain Module1");
    TEST_ASSERT(content.find("Module2") != std::string::npos, "Should contain Module2");
    TEST_ASSERT(content.find("Module3") != std::string::npos, "Should contain Module3");
    
    log::Logger::instance().shutdown();
    std::remove(logFile.c_str());
    return true;
}

TEST(Logger, ConsoleLevelToString) {
    TEST_EQUAL("DEBUG", std::string(log::levelToString(log::Level::Debug)), "Debug level string");
    TEST_EQUAL("INFO", std::string(log::levelToString(log::Level::Info)), "Info level string");
    TEST_EQUAL("WARN", std::string(log::levelToString(log::Level::Warning)), "Warning level string");
    TEST_EQUAL("ERROR", std::string(log::levelToString(log::Level::Error)), "Error level string");
    TEST_EQUAL("CRITICAL", std::string(log::levelToString(log::Level::Critical)), "Critical level string");
    
    return true;
}
