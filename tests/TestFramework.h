#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <iomanip>
#include <chrono>

/// Simple test framework for Sumo Balls
namespace test {

/// Test result
struct TestResult {
    std::string name;
    bool passed;
    std::string message;
    double duration_ms;
};

/// Test suite manager
class TestSuite {
public:
    static TestSuite& instance() {
        static TestSuite suite;
        return suite;
    }
    
    /// Register a test
    void registerTest(const std::string& name, std::function<bool(std::string&)> testFunc) {
        tests.push_back({name, testFunc});
    }
    
    /// Run all tests
    int runAll() {
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "Running " << tests.size() << " test(s)" << std::endl;
        std::cout << std::string(70, '=') << "\n" << std::endl;
        
        int passed = 0;
        int failed = 0;
        std::vector<TestResult> results;
        
        for (const auto& test : tests) {
            std::string errorMsg;
            auto start = std::chrono::high_resolution_clock::now();
            bool success = test.func(errorMsg);
            auto end = std::chrono::high_resolution_clock::now();
            
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            
            TestResult result{test.name, success, errorMsg, duration};
            results.push_back(result);
            
            if (success) {
                std::cout << "✓ " << std::left << std::setw(60) << test.name 
                         << " [" << std::fixed << std::setprecision(2) << duration << "ms]" << std::endl;
                passed++;
            } else {
                std::cout << "✗ " << std::left << std::setw(60) << test.name 
                         << " [" << std::fixed << std::setprecision(2) << duration << "ms]" << std::endl;
                std::cout << "  Error: " << errorMsg << std::endl;
                failed++;
            }
        }
        
        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "Results: " << passed << " passed, " << failed << " failed" << std::endl;
        std::cout << std::string(70, '=') << "\n" << std::endl;
        
        return failed == 0 ? 0 : 1;
    }
    
private:
    struct Test {
        std::string name;
        std::function<bool(std::string&)> func;
    };
    
    std::vector<Test> tests;
};

} // namespace test

/// Assertion macros
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        errorMsg = message; \
        return false; \
    }

#define TEST_EQUAL(expected, actual, message) \
    if ((expected) != (actual)) { \
        errorMsg = message; \
        return false; \
    }

#define TEST_TRUE(condition) TEST_ASSERT(condition, #condition " is not true")
#define TEST_FALSE(condition) TEST_ASSERT(!(condition), #condition " is not false")

/// TEST macro - simple approach using lambda registration
#define TEST(Suite, TestName) \
    static struct TestRegister_##Suite##_##TestName { \
        TestRegister_##Suite##_##TestName(); \
    } register_##Suite##_##TestName; \
    TestRegister_##Suite##_##TestName::TestRegister_##Suite##_##TestName()

#define TEST_IMPL(Suite, TestName) \
    bool test_impl_##Suite##_##TestName(std::string& errorMsg)
