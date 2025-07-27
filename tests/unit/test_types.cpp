#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Types.hpp"

using namespace FlowGraph;

TEST_CASE("Value construction and type checking", "[types][value]") {
    SECTION("Number values") {
        Value numberVal = createValue(42.0);
        REQUIRE(getValueType(numberVal) == ValueType::Number);
        REQUIRE(numberVal.isNumber());
        REQUIRE(numberVal.asNumber() == 42.0);
        
        Value floatVal = createValue(3.14);
        REQUIRE(getValueType(floatVal) == ValueType::Number);
        REQUIRE(floatVal.isNumber());
        REQUIRE(floatVal.asNumber() == 3.14);
    }
    
    SECTION("Boolean values") {
        Value boolVal = createValue(true);
        REQUIRE(getValueType(boolVal) == ValueType::Boolean);
        REQUIRE(boolVal.isBoolean());
        REQUIRE(boolVal.asBoolean() == true);
    }
    
    SECTION("String values") {
        Value stringVal = createValue("hello");
        REQUIRE(getValueType(stringVal) == ValueType::String);
        REQUIRE(stringVal.isString());
        REQUIRE(stringVal.asString() == "hello");
    }
}

TEST_CASE("Value comparison operations", "[types][value]") {
    SECTION("Number comparison") {
        Value a = createValue(10.0);
        Value b = createValue(20.0);
        Value c = createValue(10.0);
        
        REQUIRE(a == c);
        REQUIRE(a != b);
        // Note: ExpressionKit::Value comparison operators work with same types
    }
    
    SECTION("String comparison") {
        Value a = createValue("apple");
        Value b = createValue("banana");
        Value c = createValue("apple");
        
        REQUIRE(a == c);
        REQUIRE(a != b);
    }
}

TEST_CASE("Value string conversion", "[types][value]") {
    SECTION("Converting different types to string") {
        REQUIRE(createValue(42.0).toString() == "42.000000");
        REQUIRE(createValue(3.14).toString() == "3.140000"); 
        REQUIRE(createValue(true).toString() == "true");
        REQUIRE(createValue(false).toString() == "false");
        REQUIRE(createValue("hello").toString() == "hello");
    }
}

TEST_CASE("Value boolean conversion", "[types][value]") {
    SECTION("Converting different types to boolean") {
        // Numbers: 0 is false, everything else is true
        REQUIRE(createValue(0.0).asBoolean() == false);
        REQUIRE(createValue(1.0).asBoolean() == true);
        REQUIRE(createValue(-1.0).asBoolean() == true);
        REQUIRE(createValue(0.1).asBoolean() == true);
        
        // Booleans: direct conversion
        REQUIRE(createValue(true).asBoolean() == true);
        REQUIRE(createValue(false).asBoolean() == false);
        
        // Strings: empty is false, non-empty is true
        REQUIRE(createValue("").asBoolean() == false);
        REQUIRE(createValue("hello").asBoolean() == true);
        REQUIRE(createValue(" ").asBoolean() == true); // whitespace is considered non-empty
        REQUIRE(createValue("0").asBoolean() == true); // string "0" is non-empty, so true
        REQUIRE(createValue("false").asBoolean() == true); // string "false" is non-empty, so true
    }
}

TEST_CASE("Value edge cases and robustness", "[types][value]") {
    SECTION("Number precision and special values") {
        // Test very small and large numbers
        Value tiny = createValue(1e-308);
        Value huge = createValue(1e308);
        REQUIRE(tiny.isNumber());
        REQUIRE(huge.isNumber());
        
        // Test negative zero
        Value negZero = createValue(-0.0);
        REQUIRE(negZero.isNumber());
        REQUIRE(negZero.asNumber() == 0.0);
    }
    
    SECTION("String edge cases") {
        // Test null character in string
        Value nullCharString = createValue(std::string("hello\0world", 11));
        REQUIRE(nullCharString.isString());
        REQUIRE(nullCharString.asString().length() == 11);
        
        // Test very long string
        std::string longString(10000, 'x');
        Value longStringValue = createValue(longString);
        REQUIRE(longStringValue.isString());
        REQUIRE(longStringValue.asString().length() == 10000);
    }
    
    SECTION("Type consistency checks") {
        // Ensure TypeInfo matching works correctly for all types
        TypeInfo numberType(ValueType::Number);
        TypeInfo boolType(ValueType::Boolean);
        TypeInfo stringType(ValueType::String);
        
        REQUIRE(numberType.matches(createValue(42.0)));
        REQUIRE(numberType.matches(createValue(-3.14)));
        REQUIRE_FALSE(numberType.matches(createValue(true)));
        REQUIRE_FALSE(numberType.matches(createValue("text")));
        
        REQUIRE(boolType.matches(createValue(true)));
        REQUIRE(boolType.matches(createValue(false)));
        REQUIRE_FALSE(boolType.matches(createValue(1.0)));
        REQUIRE_FALSE(boolType.matches(createValue("true")));
        
        REQUIRE(stringType.matches(createValue("hello")));
        REQUIRE(stringType.matches(createValue("")));
        REQUIRE_FALSE(stringType.matches(createValue(42.0)));
        REQUIRE_FALSE(stringType.matches(createValue(false)));
    }
}

TEST_CASE("TypeInfo validation", "[types][typeinfo]") {
    SECTION("Required parameter matching") {
        TypeInfo numberType(ValueType::Number, false);
        
        REQUIRE(numberType.matches(createValue(42.0)));
        REQUIRE(numberType.matches(createValue(3.14)));
        REQUIRE_FALSE(numberType.matches(createValue("hello")));
    }
    
    SECTION("Boolean type matching") {
        TypeInfo boolType(ValueType::Boolean, false);
        
        REQUIRE(boolType.matches(createValue(true)));
        REQUIRE(boolType.matches(createValue(false)));
        REQUIRE_FALSE(boolType.matches(createValue(42.0)));
    }
    
    SECTION("Optional parameter matching") {
        TypeInfo optionalStringType(ValueType::String, true);
        
        REQUIRE(optionalStringType.matches(createValue("hello")));
        // Note: For optional parameters, we'd need to handle null/empty values
        // This will be implemented when we add optional value support
    }
}

TEST_CASE("Type string conversion", "[types][conversion]") {
    SECTION("ValueType to string") {
        REQUIRE(valueTypeToString(ValueType::Number) == "N");
        REQUIRE(valueTypeToString(ValueType::Boolean) == "B");
        REQUIRE(valueTypeToString(ValueType::String) == "S");
    }
    
    SECTION("String to ValueType") {
        REQUIRE(parseValueType("N") == ValueType::Number);
        REQUIRE(parseValueType("B") == ValueType::Boolean);
        REQUIRE(parseValueType("S") == ValueType::String);
    }
    
    SECTION("Backward compatibility for old type strings") {
        // Old "I" and "F" types should both map to Number for backward compatibility
        REQUIRE(parseValueType("I") == ValueType::Number);
        REQUIRE(parseValueType("F") == ValueType::Number);
    }
    
    SECTION("Invalid type string throws") {
        REQUIRE_THROWS_AS(parseValueType("X"), FlowGraphError);
        REQUIRE_THROWS_AS(parseValueType(""), FlowGraphError);
        REQUIRE_THROWS_AS(parseValueType("i"), FlowGraphError); // lowercase should fail
        REQUIRE_THROWS_AS(parseValueType("NUMBER"), FlowGraphError); // full name should fail
    }
    
    SECTION("Edge cases for type conversion") {
        // Test extreme numbers
        Value largeNumber = createValue(1e308);
        REQUIRE(getValueType(largeNumber) == ValueType::Number);
        
        Value smallNumber = createValue(-1e308);
        REQUIRE(getValueType(smallNumber) == ValueType::Number);
        
        Value zero = createValue(0.0);
        REQUIRE(getValueType(zero) == ValueType::Number);
        
        // Test empty string
        Value emptyString = createValue("");
        REQUIRE(getValueType(emptyString) == ValueType::String);
        REQUIRE(emptyString.asString() == "");
        
        // Test string with special characters
        Value specialString = createValue("Hello\nWorld\t🚀");
        REQUIRE(getValueType(specialString) == ValueType::String);
        REQUIRE(specialString.asString() == "Hello\nWorld\t🚀");
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
        results["output"] = createValue("success");
        
        ExecutionResult result(results);
        REQUIRE(result.success);
        REQUIRE(result.error.empty());
        REQUIRE(result.returnValues["output"].asString() == "success");
    }
    
    SECTION("ExecutionResult failure") {
        ExecutionResult result("Something went wrong");
        REQUIRE_FALSE(result.success);
        REQUIRE(result.error == "Something went wrong");
    }
}