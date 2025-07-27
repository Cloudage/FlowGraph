#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/Engine.hpp"

using namespace FlowGraph;

TEST_CASE("ExpressionKit integration - Basic evaluation", "[expression][integration]") {
    SECTION("Simple arithmetic expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test basic arithmetic - ExpressionKit returns all numbers as double
        auto result1 = context.evaluateExpression("2 + 3");
        REQUIRE(result1.type() == ValueType::Float);
        REQUIRE(result1.get<double>() == 5.0);
        
        auto result2 = context.evaluateExpression("10 - 4");
        REQUIRE(result2.type() == ValueType::Float);
        REQUIRE(result2.get<double>() == 6.0);
        
        auto result3 = context.evaluateExpression("3 * 4");
        REQUIRE(result3.type() == ValueType::Float);
        REQUIRE(result3.get<double>() == 12.0);
        
        auto result4 = context.evaluateExpression("15.0 / 3.0");
        REQUIRE(result4.type() == ValueType::Float);
        REQUIRE(result4.get<double>() == 5.0);
    }
    
    SECTION("Boolean expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("true && false");
        REQUIRE(result1.type() == ValueType::Boolean);
        REQUIRE(result1.get<bool>() == false);
        
        auto result2 = context.evaluateExpression("true || false");
        REQUIRE(result2.type() == ValueType::Boolean);
        REQUIRE(result2.get<bool>() == true);
        
        auto result3 = context.evaluateExpression("!true");
        REQUIRE(result3.type() == ValueType::Boolean);
        REQUIRE(result3.get<bool>() == false);
    }
    
    SECTION("Comparison expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("5 > 3");
        REQUIRE(result1.type() == ValueType::Boolean);
        REQUIRE(result1.get<bool>() == true);
        
        auto result2 = context.evaluateExpression("10 == 10");
        REQUIRE(result2.type() == ValueType::Boolean);
        REQUIRE(result2.get<bool>() == true);
        
        auto result3 = context.evaluateExpression("7 <= 7");
        REQUIRE(result3.type() == ValueType::Boolean);
        REQUIRE(result3.get<bool>() == true);
    }
}

TEST_CASE("ExpressionKit integration - Variable access", "[expression][integration][variables]") {
    SECTION("Basic variable access") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Set some variables
        context.setVariable("x", Value(static_cast<int64_t>(10)));
        context.setVariable("y", Value(5.5));
        context.setVariable("flag", Value(true));
        
        // Test variable access in expressions - ExpressionKit returns numbers as double
        auto result1 = context.evaluateExpression("x + 5");
        REQUIRE(result1.type() == ValueType::Float);
        REQUIRE(result1.get<double>() == 15.0);
        
        auto result2 = context.evaluateExpression("y * 2");
        REQUIRE(result2.type() == ValueType::Float);
        REQUIRE(result2.get<double>() == 11.0);
        
        auto result3 = context.evaluateExpression("flag && true");
        REQUIRE(result3.type() == ValueType::Boolean);
        REQUIRE(result3.get<bool>() == true);
    }
    
    SECTION("Variable in complex expressions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("a", Value(static_cast<int64_t>(10)));
        context.setVariable("b", Value(static_cast<int64_t>(20)));
        context.setVariable("c", Value(static_cast<int64_t>(3)));
        
        auto result = context.evaluateExpression("(a + b) / c");
        REQUIRE(result.type() == ValueType::Float);
        REQUIRE(result.get<double>() == 10.0);
    }
}

TEST_CASE("ExpressionKit integration - Mathematical functions", "[expression][integration][functions]") {
    SECTION("Built-in mathematical functions") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test min/max functions
        auto result1 = context.evaluateExpression("max(10, 5)");
        REQUIRE(result1.type() == ValueType::Float);
        REQUIRE(result1.get<double>() == 10.0);
        
        auto result2 = context.evaluateExpression("min(3.5, 7.2)");
        REQUIRE(result2.type() == ValueType::Float);
        REQUIRE(result2.get<double>() == 3.5);
        
        // Test sqrt function
        auto result3 = context.evaluateExpression("sqrt(16)");
        REQUIRE(result3.type() == ValueType::Float);
        REQUIRE(result3.get<double>() == 4.0);
        
        // Test abs function
        auto result4 = context.evaluateExpression("abs(-5)");
        REQUIRE(result4.type() == ValueType::Float);
        REQUIRE(result4.get<double>() == 5.0);
    }
    
    SECTION("Functions with variables") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("num", Value(static_cast<int64_t>(25)));
        context.setVariable("neg", Value(static_cast<int64_t>(-10)));
        
        auto result1 = context.evaluateExpression("sqrt(num)");
        REQUIRE(result1.type() == ValueType::Float);
        REQUIRE(result1.get<double>() == 5.0);
        
        auto result2 = context.evaluateExpression("abs(neg)");
        REQUIRE(result2.type() == ValueType::Float);
        REQUIRE(result2.get<double>() == 10.0);
    }
}

TEST_CASE("ExpressionKit integration - String operations", "[expression][integration][strings]") {
    SECTION("String concatenation") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result = context.evaluateExpression("\"Hello, \" + \"World!\"");
        REQUIRE(result.type() == ValueType::String);
        REQUIRE(result.get<std::string>() == "Hello, World!");
    }
    
    SECTION("String comparison") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        auto result1 = context.evaluateExpression("\"test\" == \"test\"");
        REQUIRE(result1.type() == ValueType::Boolean);
        REQUIRE(result1.get<bool>() == true);
        
        auto result2 = context.evaluateExpression("\"abc\" != \"def\"");
        REQUIRE(result2.type() == ValueType::Boolean);
        REQUIRE(result2.get<bool>() == true);
    }
    
    SECTION("String variables") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        context.setVariable("greeting", Value("Hello"));
        context.setVariable("name", Value("FlowGraph"));
        
        auto result = context.evaluateExpression("greeting + \", \" + name + \"!\"");
        REQUIRE(result.type() == ValueType::String);
        REQUIRE(result.get<std::string>() == "Hello, FlowGraph!");
    }
}

TEST_CASE("ExpressionKit integration - AST node execution", "[expression][integration][ast]") {
    SECTION("Basic expression evaluation in context") {
        FlowAST ast;
        ExecutionContext context(ast);
        
        // Test that we can evaluate expressions that would be used in AST nodes
        context.setVariable("count", Value(static_cast<int64_t>(5)));
        
        // Test an assignment-like expression
        auto result1 = context.evaluateExpression("count + 10");
        REQUIRE(result1.type() == ValueType::Float);
        REQUIRE(result1.get<double>() == 15.0);
        
        // Test a condition-like expression
        auto result2 = context.evaluateExpression("count < 10");
        REQUIRE(result2.type() == ValueType::Boolean);
        REQUIRE(result2.get<bool>() == true);
        
        // Test complex expressions
        auto result3 = context.evaluateExpression("(count * 2) > 8");
        REQUIRE(result3.type() == ValueType::Boolean);
        REQUIRE(result3.get<bool>() == true);
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

TEST_CASE("ExpressionKit integration - Type conversion", "[expression][integration][types]") {
    SECTION("FlowGraph Value to ExpressionKit Value conversion") {
        Value intVal(static_cast<int64_t>(42));
        Value floatVal(3.14);
        Value boolVal(true);
        Value stringVal("test");
        
        auto ekInt = toExpressionKitValue(intVal);
        auto ekFloat = toExpressionKitValue(floatVal);
        auto ekBool = toExpressionKitValue(boolVal);
        auto ekString = toExpressionKitValue(stringVal);
        
        REQUIRE(ekInt.isNumber());
        REQUIRE(ekInt.asNumber() == 42.0);
        
        REQUIRE(ekFloat.isNumber());
        REQUIRE(ekFloat.asNumber() == 3.14);
        
        REQUIRE(ekBool.isBoolean());
        REQUIRE(ekBool.asBoolean() == true);
        
        REQUIRE(ekString.isString());
        REQUIRE(ekString.asString() == "test");
    }
    
    SECTION("ExpressionKit Value to FlowGraph Value conversion") {
        ExpressionKit::Value ekInt(42.0);
        ExpressionKit::Value ekFloat(3.14);
        ExpressionKit::Value ekBool(true);
        ExpressionKit::Value ekString("test");
        
        auto fgInt = fromExpressionKitValue(ekInt);
        auto fgFloat = fromExpressionKitValue(ekFloat);
        auto fgBool = fromExpressionKitValue(ekBool);
        auto fgString = fromExpressionKitValue(ekString);
        
        REQUIRE(fgInt.type() == ValueType::Float);
        REQUIRE(fgInt.get<double>() == 42.0);
        
        REQUIRE(fgFloat.type() == ValueType::Float);
        REQUIRE(fgFloat.get<double>() == 3.14);
        
        REQUIRE(fgBool.type() == ValueType::Boolean);
        REQUIRE(fgBool.get<bool>() == true);
        
        REQUIRE(fgString.type() == ValueType::String);
        REQUIRE(fgString.get<std::string>() == "test");
    }
}