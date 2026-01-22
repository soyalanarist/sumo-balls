#pragma once

#include <string>
#include <map>
#include <sstream>

// Minimal JSON parser/builder (just for our coordinator API)
class SimpleJson {
public:
    std::map<std::string, std::string> data;
    
    // Parse simple JSON
    static SimpleJson parse(const std::string& json) {
        SimpleJson result;
        
        // Very simple parser - finds "key":"value" pairs
        size_t pos = 0;
        while (pos < json.length()) {
            size_t keyStart = json.find("\"", pos);
            if (keyStart == std::string::npos) break;
            
            size_t keyEnd = json.find("\"", keyStart + 1);
            if (keyEnd == std::string::npos) break;
            
            std::string key = json.substr(keyStart + 1, keyEnd - keyStart - 1);
            
            size_t valueStart = json.find(":", keyEnd);
            if (valueStart == std::string::npos) break;
            
            // Skip whitespace
            valueStart++;
            while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\t')) {
                valueStart++;
            }
            
            std::string value;
            if (json[valueStart] == '\"') {
                // String value
                size_t valueEnd = json.find("\"", valueStart + 1);
                if (valueEnd == std::string::npos) break;
                value = json.substr(valueStart + 1, valueEnd - valueStart - 1);
                pos = valueEnd + 1;
            } else {
                // Number value
                size_t valueEnd = json.find_first_of(",}", valueStart);
                if (valueEnd == std::string::npos) valueEnd = json.length();
                value = json.substr(valueStart, valueEnd - valueStart);
                // Trim whitespace
                size_t end = value.find_last_not_of(" \t\r\n");
                if (end != std::string::npos) {
                    value = value.substr(0, end + 1);
                }
                pos = valueEnd;
            }
            
            result.data[key] = value;
        }
        
        return result;
    }
    
    // Build simple JSON
    std::string build() const {
        std::ostringstream ss;
        ss << "{";
        bool first = true;
        for (const auto& pair : data) {
            if (!first) ss << ",";
            ss << "\"" << pair.first << "\":\"" << pair.second << "\"";
            first = false;
        }
        ss << "}";
        return ss.str();
    }
    
    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = data.find(key);
        return (it != data.end()) ? it->second : defaultValue;
    }
    
    int getInt(const std::string& key, int defaultValue = 0) const {
        auto it = data.find(key);
        if (it == data.end()) return defaultValue;
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    
    bool getBool(const std::string& key, bool defaultValue = false) const {
        auto it = data.find(key);
        if (it == data.end()) return defaultValue;
        return (it->second == "true" || it->second == "1");
    }
    
    std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        return get(key, defaultValue);
    }
    
    void set(const std::string& key, const std::string& value) {
        data[key] = value;
    }
    
    void set(const std::string& key, int value) {
        data[key] = std::to_string(value);
    }
    
    void set(const std::string& key, bool value) {
        data[key] = value ? "true" : "false";
    }
    
    bool has(const std::string& key) const {
        return data.find(key) != data.end();
    }
    
    std::string toString() const {
        return build();
    }
};
