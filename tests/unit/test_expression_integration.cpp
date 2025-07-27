#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Engine.hpp"

using namespace FlowGraph;

TEST_CASE("ExpressionKit integration - Basic evaluation", "[expression][integration]") {
    SECTION("Simple arithmetic expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test basic arithmetic - ExpressionKit returns all numbers as double
        auto result1 = context.evaluateExpression("2 + 3");
        REQUIRE(getValueType(result1) == ValueType::Float);
        REQUIRE(result1.asNumber() == 5.0);
        
        auto result2 = context.evaluateExpression("10 - 4");
        REQUIRE(getValueType(result2) == ValueType::Float);
        REQUIRE(result2.asNumber() == 6.0);
        
        auto result3 = context.evaluateExpression("3 * 4");
        REQUIRE(getValueType(result3) == ValueType::Float);
        REQUIRE(result3.asNumber() == 12.0);
        
        auto result4 = context.evaluateExpression("15.0 / 3.0");
        REQUIRE(getValueType(result4) == ValueType::Float);
        REQUIRE(result4.asNumber() == 5.0);
    }
    
    SECTION("Boolean expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("true && false");
        REQUIRE(getValueType(result1) == ValueType::Boolean);
        REQUIRE(result1.asBoolean() == false);
        
        auto result2 = context.evaluateExpression("true || false");
        REQUIRE(getValueType(result2) == ValueType::Boolean);
        REQUIRE(result2.asBoolean() == true);
        
        auto result3 = context.evaluateExpression("!true");
        REQUIRE(getValueType(result3) == ValueType::Boolean);
        REQUIRE(result3.asBoolean() == false);
    }
    
    SECTION("Comparison expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("5 > 3");
        REQUIRE(getValueType(result1) == ValueType::Boolean);
        REQUIRE(result1.asBoolean() == true);
        
        auto result2 = context.evaluateExpression("10 == 10");
        REQUIRE(getValueType(result2) == ValueType::Boolean);
        REQUIRE(result2.asBoolean() == true);
        
        auto result3 = context.evaluateExpression("7 <= 7");
        REQUIRE(getValueType(result3) == ValueType::Boolean);
        REQUIRE(result3.asBoolean() == true);
    }
}

TEST_CASE("ExpressionKit integration - Variable access", "[expression][integration][variables]") {
    SECTION("Basic variable access") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Set some variables
        context.setVariable("x", createValue(static_cast<int64_t>(10)));
        context.setVariable("y", createValue(5.5));
        context.setVariable("flag", createValue(true));
        
        // Test variable access in expressions - ExpressionKit returns numbers as double
        auto result1 = context.evaluateExpression("x + 5");
        REQUIRE(getValueType(result1) == ValueType::Float);
        REQUIRE(result1.asNumber() == 15.0);
        
        auto result2 = context.evaluateExpression("y * 2");
        REQUIRE(getValueType(result2) == ValueType::Float);
        REQUIRE(result2.asNumber() == 11.0);
        
        auto result3 = context.evaluateExpression("flag && true");
        REQUIRE(getValueType(result3) == ValueType::Boolean);
        REQUIRE(result3.asBoolean() == true);
    }
    
    SECTION("Variable in complex expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("a", createValue(static_cast<int64_t>(10)));
        context.setVariable("b", createValue(static_cast<int64_t>(20)));
        context.setVariable("c", createValue(static_cast<int64_t>(3)));
        
        auto result = context.evaluateExpression("(a + b) / c");
        REQUIRE(getValueType(result) == ValueType::Float);
        REQUIRE(result.asNumber() == 10.0);
    }
}

TEST_CASE("ExpressionKit integration - Mathematical functions", "[expression][integration][functions]") {
    SECTION("Built-in mathematical functions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test min/max functions
        auto result1 = context.evaluateExpression("max(10, 5)");
        REQUIRE(getValueType(result1) == ValueType::Float);
        REQUIRE(result1.asNumber() == 10.0);
        
        auto result2 = context.evaluateExpression("min(3.5, 7.2)");
        REQUIRE(getValueType(result2) == ValueType::Float);
        REQUIRE(result2.asNumber() == 3.5);
        
        // Test sqrt function
        auto result3 = context.evaluateExpression("sqrt(16)");
        REQUIRE(getValueType(result3) == ValueType::Float);
        REQUIRE(result3.asNumber() == 4.0);
        
        // Test abs function
        auto result4 = context.evaluateExpression("abs(-5)");
        REQUIRE(getValueType(result4) == ValueType::Float);
        REQUIRE(result4.asNumber() == 5.0);
    }
    
    SECTION("Functions with variables") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("num", createValue(static_cast<int64_t>(25)));
        context.setVariable("neg", createValue(static_cast<int64_t>(-10)));
        
        auto result1 = context.evaluateExpression("sqrt(num)");
        REQUIRE(getValueType(result1) == ValueType::Float);
        REQUIRE(result1.asNumber() == 5.0);
        
        auto result2 = context.evaluateExpression("abs(neg)");
        REQUIRE(getValueType(result2) == ValueType::Float);
        REQUIRE(result2.asNumber() == 10.0);
    }
}

TEST_CASE("ExpressionKit integration - String operations", "[expression][integration][strings]") {
    SECTION("String concatenation") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result = context.evaluateExpression("\"Hello, \" + \"World!\"");
        REQUIRE(getValueType(result) == ValueType::String);
        REQUIRE(result.asString() == "Hello, World!");
    }
    
    SECTION("String comparison") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("\"test\" == \"test\"");
        REQUIRE(getValueType(result1) == ValueType::Boolean);
        REQUIRE(result1.asBoolean() == true);
        
        auto result2 = context.evaluateExpression("\"abc\" != \"def\"");
        REQUIRE(getValueType(result2) == ValueType::Boolean);
        REQUIRE(result2.asBoolean() == true);
    }
    
    SECTION("String variables") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("greeting", createValue("Hello"));
        context.setVariable("name", createValue("FlowGraph"));
        
        auto result = context.evaluateExpression("greeting + \", \" + name + \"!\"");
        REQUIRE(getValueType(result) == ValueType::String);
        REQUIRE(result.asString() == "Hello, FlowGraph!");
    }
}

TEST_CASE("ExpressionKit integration - AST node execution", "[expression][integration][ast]") {
    SECTION("Basic expression evaluation in context") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test that we can evaluate expressions that would be used in AST nodes
        context.setVariable("count", createValue(static_cast<int64_t>(5)));
        
        // Test an assignment-like expression
        auto result1 = context.evaluateExpression("count + 10");
        REQUIRE(getValueType(result1) == ValueType::Float);
        REQUIRE(result1.asNumber() == 15.0);
        
        // Test a condition-like expression
        auto result2 = context.evaluateExpression("count < 10");
        REQUIRE(getValueType(result2) == ValueType::Boolean);
        REQUIRE(result2.asBoolean() == true);
        
        // Test complex expressions
        auto result3 = context.evaluateExpression("(count * 2) > 8");
        REQUIRE(getValueType(result3) == ValueType::Boolean);
        REQUIRE(result3.asBoolean() == true);
    }
    
    SECTION("Flow execution with basic AST") {
        // Create a simple flow AST
        auto ast = std::make_unique<FlowAST>();
        ast->title = "Test Flow";
        
        // Add a simple assign node
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "result", "5 + 3");
        ast->nodes.push_back(std::move(assign));
        
        // Add flow connections
        ast->connections.emplace_back("START", "10");
        ast->connections.emplace_back("10", "END");
        
        // Create and execute flow
        Flow flow(std::move(ast));
        
        ParameterMap params;
        auto result = flow.execute(params);
        
        REQUIRE(result.success == true);
    }
}

TEST_CASE("ExpressionKit integration - Error handling", "[expression][integration][errors]") {
    SECTION("Invalid expression syntax") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        REQUIRE_THROWS_AS(context.evaluateExpression("2 + + 3"), FlowGraphError);
    }
    
    SECTION("Unknown variable") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        REQUIRE_THROWS_AS(context.evaluateExpression("unknown_var + 5"), FlowGraphError);
    }
    
    SECTION("Division by zero") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        REQUIRE_THROWS_AS(context.evaluateExpression("5 / 0"), FlowGraphError);
    }
}

TEST_CASE("ExpressionKit integration - Direct Value usage", "[expression][integration][types]") {
    SECTION("Using ExpressionKit::Value directly as FlowGraph::Value") {
        // Test that FlowGraph::Value is now ExpressionKit::Value
        Value intVal = createValue(42.0);
        Value floatVal = createValue(3.14);
        Value boolVal = createValue(true);
        Value stringVal = createValue("test");
        
        // Test that these values work directly with ExpressionKit methods
        REQUIRE(intVal.isNumber());
        REQUIRE(intVal.asNumber() == 42.0);
        
        REQUIRE(floatVal.isNumber());
        REQUIRE(floatVal.asNumber() == 3.14);
        
        REQUIRE(boolVal.isBoolean());
        REQUIRE(boolVal.asBoolean() == true);
        
        REQUIRE(stringVal.isString());
        REQUIRE(stringVal.asString() == "test");
        
        // Test that getValueType helper works
        REQUIRE(getValueType(intVal) == ValueType::Float);
        REQUIRE(getValueType(floatVal) == ValueType::Float);
        REQUIRE(getValueType(boolVal) == ValueType::Boolean);
        REQUIRE(getValueType(stringVal) == ValueType::String);
    }
}