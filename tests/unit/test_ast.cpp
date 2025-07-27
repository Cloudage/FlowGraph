#include <catch2/catch_test_macros.hpp>
#include "flowgraph/detail/AST.hpp"

using namespace FlowGraph;

TEST_CASE("AST node creation", "[ast][nodes]") {
    SECTION("AssignNode creation") {
        Location loc("test.flow", 10, 5);
        TypeInfo intType(ValueType::Integer);
        AssignNode node("10", intType, "count", "0", loc);
        
        REQUIRE(node.id == "10");
        REQUIRE(node.targetType.type == ValueType::Integer);
        REQUIRE(node.variableName == "count");
        REQUIRE(node.expression == "0");
        REQUIRE(node.location.line == 10);
    }
    
    SECTION("CondNode creation") {
        CondNode node("20", "count < 10");
        
        REQUIRE(node.id == "20");
        REQUIRE(node.condition == "count < 10");
    }
    
    SECTION("ProcNode creation and bindings") {
        ProcNode node("30", "validate_input");
        node.addBinding("data", "input", false);  // data>>input
        node.addBinding("result", "output", true); // result<<output
        
        REQUIRE(node.id == "30");
        REQUIRE(node.procedureName == "validate_input");
        REQUIRE(node.bindings.size() == 2);
        
        const auto& binding1 = node.bindings[0];
        REQUIRE(binding1.localVar == "data");
        REQUIRE(binding1.procParam == "input");
        REQUIRE_FALSE(binding1.isOutput);
        
        const auto& binding2 = node.bindings[1];
        REQUIRE(binding2.localVar == "result");
        REQUIRE(binding2.procParam == "output");
        REQUIRE(binding2.isOutput);
    }
}

TEST_CASE("FlowConnection", "[ast][connection]") {
    SECTION("Basic connection") {
        FlowConnection conn("10", "20");
        
        REQUIRE(conn.fromNode == "10");
        REQUIRE(conn.toNode == "20");
        REQUIRE(conn.fromPort.empty());
        REQUIRE(conn.toPort.empty());
    }
    
    SECTION("Conditional connection") {
        FlowConnection conn("30", "40", "Y");
        
        REQUIRE(conn.fromNode == "30");
        REQUIRE(conn.toNode == "40");
        REQUIRE(conn.fromPort == "Y");
    }
}

TEST_CASE("FlowAST operations", "[ast][flow]") {
    SECTION("Find node by ID") {
        FlowAST ast;
        
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "count", "0");
        auto cond = std::make_unique<CondNode>("20", "count < 10");
        
        ast.nodes.push_back(std::move(assign));
        ast.nodes.push_back(std::move(cond));
        
        FlowNode* found = ast.findNode("10");
        REQUIRE(found != nullptr);
        REQUIRE(found->id == "10");
        
        FlowNode* notFound = ast.findNode("99");
        REQUIRE(notFound == nullptr);
    }
    
    SECTION("Get connections from node") {
        FlowAST ast;
        ast.connections.emplace_back("10", "20");
        ast.connections.emplace_back("10", "30", "Y");
        ast.connections.emplace_back("20", "40");
        
        auto fromNode10 = ast.getConnectionsFrom("10");
        REQUIRE(fromNode10.size() == 2);
        REQUIRE(fromNode10[0].toNode == "20");
        REQUIRE(fromNode10[1].toNode == "30");
        REQUIRE(fromNode10[1].fromPort == "Y");
        
        auto fromNode20 = ast.getConnectionsFrom("20");
        REQUIRE(fromNode20.size() == 1);
        REQUIRE(fromNode20[0].toNode == "40");
    }
    
    SECTION("Get connections to node") {
        FlowAST ast;
        ast.connections.emplace_back("10", "20");
        ast.connections.emplace_back("30", "20");
        ast.connections.emplace_back("20", "40");
        
        auto toNode20 = ast.getConnectionsTo("20");
        REQUIRE(toNode20.size() == 2);
        REQUIRE(toNode20[0].fromNode == "10");
        REQUIRE(toNode20[1].fromNode == "30");
        
        auto toNode40 = ast.getConnectionsTo("40");
        REQUIRE(toNode40.size() == 1);
        REQUIRE(toNode40[0].fromNode == "20");
    }
}

TEST_CASE("FlowAST validation", "[ast][validation]") {
    SECTION("Valid flow passes validation") {
        FlowAST ast;
        ast.title = "Test Flow";
        
        // Add a simple node
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "count", "0");
        ast.nodes.push_back(std::move(assign));
        
        // Add valid connections
        ast.connections.emplace_back("START", "10");
        ast.connections.emplace_back("10", "END");
        
        auto errors = ast.validate();
        REQUIRE(errors.empty());
    }
    
    SECTION("Missing START connection") {
        FlowAST ast;
        
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "count", "0");
        ast.nodes.push_back(std::move(assign));
        ast.connections.emplace_back("10", "END");
        
        auto errors = ast.validate();
        REQUIRE(errors.size() == 1);
        REQUIRE(errors[0] == "Flow must have a START connection");
    }
    
    SECTION("Missing END connection") {
        FlowAST ast;
        
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "count", "0");
        ast.nodes.push_back(std::move(assign));
        ast.connections.emplace_back("START", "10");
        
        auto errors = ast.validate();
        REQUIRE(errors.size() == 1);
        REQUIRE(errors[0] == "Flow must have at least one END connection");
    }
    
    SECTION("Unknown node reference") {
        FlowAST ast;
        
        auto assign = std::make_unique<AssignNode>("10", TypeInfo(ValueType::Integer), "count", "0");
        ast.nodes.push_back(std::move(assign));
        ast.connections.emplace_back("START", "10");
        ast.connections.emplace_back("10", "20"); // 20 doesn't exist
        ast.connections.emplace_back("20", "END");
        
        auto errors = ast.validate();
        REQUIRE(errors.size() == 2); // 10->20 and 20->END both reference unknown node 20
        REQUIRE(std::find(errors.begin(), errors.end(), "Connection references unknown node: 20") != errors.end());
    }
}