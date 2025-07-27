#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Parser.hpp"
#include <iostream>

using namespace FlowGraph;

TEST_CASE("Basic parser functionality", "[parser]") {
    SECTION("Parse hello world example") {
        std::string content = R"(
TITLE: Hello World

NODES:
10 PROC print msg>>"Hello, FlowGraph!"

FLOW:
START -> 10
10 -> END
)";
        
        Parser parser;
        auto ast = parser.parse(content, "hello.flow");
        
        REQUIRE(ast != nullptr);
        REQUIRE(ast->title == "Hello World");
        REQUIRE(ast->errors.empty()); // No errors defined
    }
}

TEST_CASE("Error handling parser functionality", "[parser][errors]") {
    SECTION("Parse ERRORS section") {
        std::string content = R"(
TITLE: Test Flow

ERRORS:
VALIDATION_ERROR
PROCESSING_ERROR

NODES:
10 ASSIGN I count 0

FLOW:
START -> 10
10 -> END
)";
        
        Parser parser;
        auto ast = parser.parse(content, "test.flow");
        
        REQUIRE(ast != nullptr);
        REQUIRE(ast->title == "Test Flow");
        REQUIRE(ast->errors.size() == 2);
        REQUIRE(ast->hasError("VALIDATION_ERROR"));
        REQUIRE(ast->hasError("PROCESSING_ERROR"));
        REQUIRE_FALSE(ast->hasError("NON_EXISTENT"));
    }
    
    SECTION("Parse complete flow with error handling") {
        std::string content = R"(
TITLE: User Authentication

PARAMS:
S username
S password

RETURNS:
B success
S token

ERRORS:
USER_NOT_FOUND
INVALID_PASSWORD
AUTH_SERVICE_ERROR

NODES:
10 PROC check_user username>>login exists<<found
20 COND found
30 PROC verify_password password>>input username>>user valid<<is_valid
40 COND is_valid
50 PROC generate_token username>>user token<<auth_token
60 ASSIGN B success true

100 ASSIGN B success false
110 ASSIGN S token ""

FLOW:
START -> 10
10 -> 20
10.USER_NOT_FOUND -> 100
20.Y -> 30
20.N -> USER_NOT_FOUND
30 -> 40
30.AUTH_SERVICE_ERROR -> 100
40.Y -> 50
40.N -> INVALID_PASSWORD
50 -> 60
60 -> END
100 -> 110
110 -> END
)";
        
        Parser parser;
        auto ast = parser.parse(content, "auth_test.flow");
        
        REQUIRE(ast != nullptr);
        REQUIRE(ast->title == "User Authentication");
        REQUIRE(ast->errors.size() == 3);
        REQUIRE(ast->hasError("USER_NOT_FOUND"));
        REQUIRE(ast->hasError("INVALID_PASSWORD"));
        REQUIRE(ast->hasError("AUTH_SERVICE_ERROR"));
    }
}

// More comprehensive parser tests will be added in next iteration

TEST_CASE("Complete error handling integration", "[parser][errors][integration]") {
    SECTION("Full error handling workflow") {
        std::string content = R"(
// Error handling integration test
TITLE: Complete Error Demo

PARAMS:
S input_data
I retry_count

RETURNS:
B success
S result
S error_message

ERRORS:
VALIDATION_FAILED
NETWORK_ERROR
TIMEOUT_ERROR
RETRY_EXCEEDED

NODES:
10 PROC validate input_data>>data valid<<is_valid
20 COND is_valid
30 PROC process_data input_data>>data result<<output retry_count>>retries
40 COND retry_count > 3
50 ASSIGN B success true
60 ASSIGN S result output
70 ASSIGN S error_message ""

100 ASSIGN B success false
110 ASSIGN S result ""
120 ASSIGN S error_message "Validation failed"

200 ASSIGN B success false
210 ASSIGN S result ""
220 ASSIGN S error_message "Too many retries"

FLOW:
START -> 10
10 -> 20
10.VALIDATION_FAILED -> 100
20.Y -> 30
20.N -> VALIDATION_FAILED
30 -> 40
30.NETWORK_ERROR -> 40
30.TIMEOUT_ERROR -> 40
40.Y -> 200
40.N -> 50
50 -> 60
60 -> 70
70 -> END
100 -> 110
110 -> 120
120 -> END
200 -> 210
210 -> 220
220 -> END
)";
        
        Parser parser;
        auto ast = parser.parse(content, "integration_test.flow");
        
        REQUIRE(ast != nullptr);
        REQUIRE(ast->title == "Complete Error Demo");
        
        // Verify error definitions
        REQUIRE(ast->errors.size() == 4);
        REQUIRE(ast->hasError("VALIDATION_FAILED"));
        REQUIRE(ast->hasError("NETWORK_ERROR"));
        REQUIRE(ast->hasError("TIMEOUT_ERROR"));
        REQUIRE(ast->hasError("RETRY_EXCEEDED"));
        
        // Verify error names are exactly as expected
        std::vector<std::string> expectedErrors = {
            "VALIDATION_FAILED", "NETWORK_ERROR", "TIMEOUT_ERROR", "RETRY_EXCEEDED"
        };
        
        for (size_t i = 0; i < expectedErrors.size(); ++i) {
            REQUIRE(ast->errors[i].name == expectedErrors[i]);
        }
        
        std::cout << "\n✓ Error handling integration test completed successfully!" << std::endl;
        std::cout << "  - Parsed " << ast->errors.size() << " error definitions" << std::endl;
        std::cout << "  - All error checks passed" << std::endl;
        std::cout << "  - Ready for error flow execution (when engine is implemented)" << std::endl;
    }
}