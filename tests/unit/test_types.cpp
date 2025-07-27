#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Types.hpp"

using namespace FlowGraph;

TEST_CASE("Value construction and type checking", "[types][value]") {
    SECTION("Integer values") {
        Value intVal(static_cast<int64_t>(42));
        REQUIRE(intVal.type() == ValueType::Integer);
        REQUIRE(intVal.holds<int64_t>());
        REQUIRE(intVal.get<int64_t>() == 42);
    }
    
    SECTION("Float values") {
        Value floatVal(3.14);
        REQUIRE(floatVal.type() == ValueType::Float);
        REQUIRE(floatVal.holds<double>());
        REQUIRE(floatVal.get<double>() == 3.14);
    }
    
    SECTION("Boolean values") {
        Value boolVal(true);
        REQUIRE(boolVal.type() == ValueType::Boolean);
        REQUIRE(boolVal.holds<bool>());
        REQUIRE(boolVal.get<bool>() == true);
    }
    
    SECTION("String values") {
        Value stringVal("hello");
        REQUIRE(stringVal.type() == ValueType::String);
        REQUIRE(stringVal.holds<std::string>());
        REQUIRE(stringVal.get<std::string>() == "hello");
    }
}

TEST_CASE("Value comparison operations", "[types][value]") {
    SECTION("Integer comparison") {
        Value a(static_cast<int64_t>(10));
        Value b(static_cast<int64_t>(20));
        Value c(static_cast<int64_t>(10));
        
        REQUIRE(a == c);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(b > a);
        REQUIRE(a <= c);
        REQUIRE(a >= c);
    }
    
    SECTION("String comparison") {
        Value a("apple");
        Value b("banana");
        Value c("apple");
        
        REQUIRE(a == c);
        REQUIRE(a != b);
        REQUIRE(a < b);
    }
}

TEST_CASE("Value string conversion", "[types][value]") {
    SECTION("Converting different types to string") {
        REQUIRE(Value(static_cast<int64_t>(42)).toString() == "42");
        REQUIRE(Value(3.14).toString() == "3.140000");
        REQUIRE(Value(true).toString() == "true");
        REQUIRE(Value(false).toString() == "false");
        REQUIRE(Value("hello").toString() == "hello");
    }
}

TEST_CASE("Value boolean conversion", "[types][value]") {
    SECTION("Converting different types to boolean") {
        // Integers: 0 is false, everything else is true
        REQUIRE(Value(static_cast<int64_t>(0)).toBool() == false);
        REQUIRE(Value(static_cast<int64_t>(1)).toBool() == true);
        REQUIRE(Value(static_cast<int64_t>(-1)).toBool() == true);
        
        // Floats: 0.0 is false, everything else is true
        REQUIRE(Value(0.0).toBool() == false);
        REQUIRE(Value(0.1).toBool() == true);
        REQUIRE(Value(-0.1).toBool() == true);
        
        // Booleans: direct conversion
        REQUIRE(Value(true).toBool() == true);
        REQUIRE(Value(false).toBool() == false);
        
        // Strings: empty is false, non-empty is true
        REQUIRE(Value("").toBool() == false);
        REQUIRE(Value("hello").toBool() == true);
        REQUIRE(Value(" ").toBool() == true); // whitespace is considered non-empty
    }
}

TEST_CASE("TypeInfo validation", "[types][typeinfo]") {
    SECTION("Required parameter matching") {
        TypeInfo intType(ValueType::Integer, false);
        
        REQUIRE(intType.matches(Value(static_cast<int64_t>(42))));
        REQUIRE_FALSE(intType.matches(Value(3.14)));
        REQUIRE_FALSE(intType.matches(Value("hello")));
    }
    
    SECTION("Optional parameter matching") {
        TypeInfo optionalStringType(ValueType::String, true);
        
        REQUIRE(optionalStringType.matches(Value("hello")));
        // Note: For optional parameters, we'd need to handle null/empty values
        // This will be implemented when we add optional value support
    }
}

TEST_CASE("Type string conversion", "[types][conversion]") {
    SECTION("ValueType to string") {
        REQUIRE(valueTypeToString(ValueType::Integer) == "I");
        REQUIRE(valueTypeToString(ValueType::Float) == "F");
        REQUIRE(valueTypeToString(ValueType::Boolean) == "B");
        REQUIRE(valueTypeToString(ValueType::String) == "S");
    }
    
    SECTION("String to ValueType") {
        REQUIRE(parseValueType("I") == ValueType::Integer);
        REQUIRE(parseValueType("F") == ValueType::Float);
        REQUIRE(parseValueType("B") == ValueType::Boolean);
        REQUIRE(parseValueType("S") == ValueType::String);
    }
    
    SECTION("Invalid type string throws") {
        REQUIRE_THROWS_AS(parseValueType("X"), FlowGraphError);
    }
}

TEST_CASE("FlowGraph error handling", "[types][error]") {
    SECTION("Error construction and properties") {
        Location loc("test.flow", 10, 5);
        FlowGraphError error(FlowGraphError::Type::Parse, "Test error", loc);
        
        REQUIRE(error.type() == FlowGraphError::Type::Parse);
        REQUIRE(error.message() == "Test error");
        REQUIRE(error.location().has_value());
        REQUIRE(error.location()->line == 10);
        REQUIRE(error.location()->column == 5);
    }
    
    SECTION("Error without location") {
        FlowGraphError error(FlowGraphError::Type::Runtime, "Runtime error");
        
        REQUIRE(error.type() == FlowGraphError::Type::Runtime);
        REQUIRE(error.message() == "Runtime error");
        REQUIRE_FALSE(error.location().has_value());
    }
}

TEST_CASE("Parameter and ExecutionResult", "[types][parameter]") {
    SECTION("Parameter construction") {
        Parameter param("test_param", TypeInfo(ValueType::String), "Test parameter");
        
        REQUIRE(param.name == "test_param");
        REQUIRE(param.type.type == ValueType::String);
        REQUIRE(param.comment == "Test parameter");
    }
    
    SECTION("ExecutionResult success") {
        ParameterMap results;
        results["output"] = Value("success");
        
        ExecutionResult result(results);
        REQUIRE(result.success);
        REQUIRE(result.error.empty());
        REQUIRE(result.returnValues["output"].get<std::string>() == "success");
    }
    
    SECTION("ExecutionResult failure") {
        ExecutionResult result("Something went wrong");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.error == "Something went wrong");
    }
}