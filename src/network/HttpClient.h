#pragma once

#include <string>
#include <map>
#include <sstream>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>

// Simple HTTP client using raw sockets (no external dependencies)
class HttpClient {
public:
    static std::string post(const std::string& url, const std::string& jsonBody, const std::string& authToken = "") {
        return request("POST", url, jsonBody, authToken);
    }
    
    static std::string get(const std::string& url, const std::string& authToken = "") {
        return request("GET", url, "", authToken);
    }
    
private:
    static std::string request(const std::string& method, const std::string& url, const std::string& body, const std::string& authToken = "") {
        // Parse URL: http://host:port/path
        std::string host = "localhost";
        int port = 8888;
        std::string path = "/";
        
        size_t hostStart = url.find("://");
        if (hostStart != std::string::npos) {
            hostStart += 3;
            size_t pathStart = url.find("/", hostStart);
            std::string hostPort = url.substr(hostStart, pathStart - hostStart);
            
            size_t colonPos = hostPort.find(":");
            if (colonPos != std::string::npos) {
                host = hostPort.substr(0, colonPos);
                port = std::stoi(hostPort.substr(colonPos + 1));
            } else {
                host = hostPort;
            }
            
            if (pathStart != std::string::npos) {
                path = url.substr(pathStart);
            }
        }
        
        // Create socket
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            return "{\"error\":\"socket creation failed\"}";
        }
        
        // Set socket timeout to 2 seconds
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
        
        // Connect to server - resolve hostname using getaddrinfo
        struct addrinfo hints, *res = nullptr;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        std::string portStr = std::to_string(port);
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0) {
            close(sockfd);
            return "{\"error\":\"hostname resolution failed\"}";
        }
        
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
            freeaddrinfo(res);
            close(sockfd);
            return "{\"error\":\"connection failed\"}";
        }
        
        freeaddrinfo(res);
        
        // Build HTTP request
        std::ostringstream request;
        request << method << " " << path << " HTTP/1.1\r\n";
        request << "Host: " << host << ":" << port << "\r\n";
        request << "Content-Type: application/json\r\n";
        request << "Content-Length: " << body.length() << "\r\n";
        
        // Add auth header if token provided
        if (!authToken.empty()) {
            request << "Authorization: Bearer " << authToken << "\r\n";
        }
        
        request << "Connection: close\r\n";
        request << "\r\n";
        if (!body.empty()) {
            request << body;
        }
        
        std::string requestStr = request.str();
        if (send(sockfd, requestStr.c_str(), requestStr.length(), 0) < 0) {
            close(sockfd);
            return "{\"error\":\"send failed\"}";
        }
        
        // Read response
        char buffer[4096];
        std::string response;
        int bytesRead;
        while ((bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
        
        close(sockfd);
        
        // Extract JSON body from HTTP response
        size_t bodyStart = response.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            return response.substr(bodyStart + 4);
        }
        
        return response;
    }
};
