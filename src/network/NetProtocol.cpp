#include "NetProtocol.h"
#include <sstream>

namespace net {

std::string ParseResult::getErrorMessage() const {
    std::ostringstream oss;
    
    switch (error) {
        case ParseError::Success:
            oss << "[OK] " << context;
            break;
            
        case ParseError::PacketTooShort:
            oss << "[ERROR] Packet too short: expected " << expectedBytes 
                << " bytes, received " << actualBytes << " bytes. " << context;
            break;
            
        case ParseError::InvalidProtocolVersion:
            oss << "[ERROR] Protocol version mismatch: expected v" 
                << static_cast<int>(expectedVersion) 
                << ", received v" << static_cast<int>(actualVersion) 
                << ". " << context;
            break;
            
        case ParseError::InvalidFieldValue:
            oss << "[ERROR] Invalid field value in '" << fieldName << "': " << context;
            break;
            
        case ParseError::CorruptedData:
            oss << "[ERROR] Data corruption detected: " << context;
            break;
            
        case ParseError::UnknownMessageType:
            oss << "[ERROR] Unknown message type. " << context;
            break;
            
        case ParseError::InvalidPlayerCount:
            oss << "[ERROR] Invalid player count. " << context;
            break;
            
        case ParseError::InvalidMessageStructure:
            oss << "[ERROR] Invalid message structure. " << context;
            break;
            
        default:
            oss << "[ERROR] Unknown parse error. " << context;
            break;
    }
    
    return oss.str();
}

} // namespace net
