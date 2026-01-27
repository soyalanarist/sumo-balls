#pragma once

#include <string>
#include <map>
#include <vector>
#include <sstream>

// Simple JSON parser/builder for coordinator API
// Handles flat key-value pairs and nested objects (as JSON strings)
class SimpleJson {
public:
    std::map<std::string, std::string> data;

    // Basic JSON string unescape (handles \n, \t, \", \\, and \uXXXX)
    // Enough for coordinator responses (not a full JSON implementation)
    static std::string unescape(const std::string& raw) {
        std::string out;
        out.reserve(raw.size());
        for (size_t i = 0; i < raw.size(); ++i) {
            char c = raw[i];
            if (c != '\\') {
                out.push_back(c);
                continue;
            }

            // Escape sequence
            if (i + 1 >= raw.size()) {
                break; // malformed escape
            }
            char esc = raw[++i];
            switch (esc) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    // Expect 4 hex digits
                    if (i + 4 >= raw.size()) {
                        break; // malformed
                    }
                    unsigned int code = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = raw[i + 1 + k];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= (h - '0');
                        else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                        else { code = 0; break; }
                    }
                    i += 4;
                    // Only handle BMP -> single byte subset we actually need (e.g., \u0026 -> '&')
                    if (code <= 0x7F) {
                        out.push_back(static_cast<char>(code));
                    }
                    break;
                }
                default:
                    out.push_back(esc); // pass through unknown escape
                    break;
            }
        }
        return out;
    }
    
    // Parse simple JSON - extracts flat key-value pairs
    // Nested objects are stored as JSON strings
    static SimpleJson parse(const std::string& json) {
        SimpleJson result;
        
        // Remove whitespace and validate basic structure
        std::string trimmed = json;
        // Find first { and last }
        size_t start = trimmed.find('{');
        size_t end = trimmed.rfind('}');
        if (start != std::string::npos && end != std::string::npos) {
            trimmed = trimmed.substr(start + 1, end - start - 1);
        }
        
        size_t pos = 0;
        while (pos < trimmed.length()) {
            // Find next key
            size_t keyStart = trimmed.find("\"", pos);
            if (keyStart == std::string::npos) break;
            
            size_t keyEnd = trimmed.find("\"", keyStart + 1);
            if (keyEnd == std::string::npos) break;
            
            std::string key = trimmed.substr(keyStart + 1, keyEnd - keyStart - 1);
            
            // Find colon
            size_t colonPos = trimmed.find(":", keyEnd);
            if (colonPos == std::string::npos) break;
            
            // Skip whitespace after colon
            size_t valueStart = colonPos + 1;
            while (valueStart < trimmed.length() && 
                   (trimmed[valueStart] == ' ' || trimmed[valueStart] == '\t' || trimmed[valueStart] == '\n')) {
                valueStart++;
            }
            
            if (valueStart >= trimmed.length()) break;
            
            std::string value;
            
            // Handle different value types
            if (trimmed[valueStart] == '\"') {
                // String value: "value"
                size_t valueEnd = trimmed.find("\"", valueStart + 1);
                if (valueEnd == std::string::npos) break;
                value = unescape(trimmed.substr(valueStart + 1, valueEnd - valueStart - 1));
                pos = valueEnd + 1;
            } else if (trimmed[valueStart] == '{') {
                // Nested object: store as JSON string
                int braceCount = 1;
                size_t i = valueStart + 1;
                while (i < trimmed.length() && braceCount > 0) {
                    if (trimmed[i] == '{') braceCount++;
                    else if (trimmed[i] == '}') braceCount--;
                    i++;
                }
                if (braceCount == 0) {
                    value = trimmed.substr(valueStart, i - valueStart);
                    pos = i;
                } else {
                    break;
                }
            } else if (trimmed[valueStart] == '[') {
                // Array: store as JSON string
                size_t i = trimmed.find("]", valueStart);
                if (i != std::string::npos) {
                    value = trimmed.substr(valueStart, i - valueStart + 1);
                    pos = i + 1;
                } else {
                    break;
                }
            } else {
                // Number, boolean, or null
                size_t valueEnd = trimmed.find_first_of(",}", valueStart);
                if (valueEnd == std::string::npos) valueEnd = trimmed.length();
                value = trimmed.substr(valueStart, valueEnd - valueStart);
                
                // Trim trailing whitespace
                size_t end = value.find_last_not_of(" \t\r\n");
                if (end != std::string::npos) {
                    value = value.substr(0, end + 1);
                }
                pos = valueEnd;
            }
            
            result.data[key] = value;
            
            // Skip to next comma or end
            pos = trimmed.find(",", pos);
            if (pos == std::string::npos) break;
            pos++;
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
    
    // Parse array of objects (returns vector of SimpleJson)
    std::vector<SimpleJson> getArray(const std::string& key) const {
        std::vector<SimpleJson> result;
        std::string arrayStr = get(key);
        
        if (arrayStr.empty() || arrayStr[0] != '[') {
            return result;
        }
        
        // Remove [ and ]
        if (arrayStr.length() >= 2) {
            arrayStr = arrayStr.substr(1, arrayStr.length() - 2);
        }
        
        // Split by objects
        int braceCount = 0;
        size_t objStart = 0;
        bool inObject = false;
        
        for (size_t i = 0; i < arrayStr.length(); ++i) {
            if (arrayStr[i] == '{') {
                if (!inObject) {
                    objStart = i;
                    inObject = true;
                }
                braceCount++;
            } else if (arrayStr[i] == '}') {
                braceCount--;
                if (braceCount == 0 && inObject) {
                    std::string objStr = arrayStr.substr(objStart, i - objStart + 1);
                    result.push_back(SimpleJson::parse(objStr));
                    inObject = false;
                }
            }
        }
        
        return result;
    }
    
    std::string toString() const {
        return build();
    }
};
