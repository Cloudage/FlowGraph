#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Parser.hpp"

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