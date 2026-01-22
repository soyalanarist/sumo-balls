#include "../TestFramework.h"
#include "../../src/network/NetProtocol.h"

TEST(NetProtocol, ParseError_Success) {
    auto result = net::ParseResult::success("Test");
    TEST_TRUE(result.isSuccess());
    TEST_EQUAL(net::ParseError::Success, result.error, "Success result should have Success error");
    return true;
}

TEST(NetProtocol, ParseError_Failure) {
    auto result = net::ParseResult::makeError(
        net::ParseError::PacketTooShort,
        "Expected more bytes"
    );
    
    TEST_FALSE(result.isSuccess());
    TEST_EQUAL(net::ParseError::PacketTooShort, result.error, "Should have correct error type");
    return true;
}

TEST(NetProtocol, ErrorMessageFormatting) {
    auto result = net::ParseResult::makeError(
        net::ParseError::InvalidFieldValue,
        "Radius out of bounds"
    );
    
    std::string msg = result.getErrorMessage();
    TEST_ASSERT(!msg.empty(), "Error message should not be empty");
    return true;
}

TEST(NetProtocol, AllErrorTypesHaveMessages) {
    // Test that all error types can generate messages
    TEST_ASSERT(!net::ParseResult::makeError(net::ParseError::PacketTooShort, "").getErrorMessage().empty(),
                "PacketTooShort should have message");
    
    TEST_ASSERT(!net::ParseResult::makeError(net::ParseError::InvalidFieldValue, "").getErrorMessage().empty(),
                "InvalidFieldValue should have message");
    
    TEST_ASSERT(!net::ParseResult::makeError(net::ParseError::InvalidPlayerCount, "").getErrorMessage().empty(),
                "InvalidPlayerCount should have message");
    
    TEST_ASSERT(!net::ParseResult::makeError(net::ParseError::CorruptedData, "").getErrorMessage().empty(),
                "CorruptedData should have message");
    
    return true;
}

TEST(NetProtocol, ParseResultContext) {
    auto result = net::ParseResult::makeError(
        net::ParseError::InvalidPlayerCount,
        "Player count mismatch"
    );
    
    // Verify context is preserved
    TEST_EQUAL(net::ParseError::InvalidPlayerCount, result.error, "Error type preserved");
    return true;
}

TEST(NetProtocol, UnknownMessageType) {
    auto result = net::ParseResult::makeError(
        net::ParseError::UnknownMessageType,
        "Unknown type 255"
    );
    
    TEST_FALSE(result.isSuccess());
    return true;
}
