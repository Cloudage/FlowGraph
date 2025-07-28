#include <iostream>
#include <flowgraph/FlowGraph.hpp>

int main() {
    std::cout << "=== FlowGraph find_package Integration Example ===" << std::endl;
    
    try {
        // Create FlowGraph engine
        FlowGraph::FlowGraphEngine engine;
        std::cout << "✓ FlowGraph engine created successfully" << std::endl;
        
        // Test ExpressionKit integration with various types
        ExpressionKit::Value numberValue(123.456);
        ExpressionKit::Value stringValue("find_package works!");
        ExpressionKit::Value boolValue(true);
        
        std::cout << "✓ ExpressionKit values:" << std::endl;
        std::cout << "  - Number: " << numberValue.asString() << std::endl;
        std::cout << "  - String: " << stringValue.asString() << std::endl;
        std::cout << "  - Boolean: " << (boolValue.asBool() ? "true" : "false") << std::endl;
        
        // Test expression evaluation
        FlowGraph::FlowAST ast;
        FlowGraph::ExecutionContext context(ast);
        
        // Set some variables in the context
        context.setVariable("x", ExpressionKit::Value(10.0));
        context.setVariable("y", ExpressionKit::Value(5.0));
        
        auto result = context.evaluateExpression("x * y + 2");
        std::cout << "✓ Expression 'x * y + 2' with x=10, y=5: " << result.asString() << std::endl;
        
        // Create AST for a simple flow
        
        auto assign = std::make_shared<FlowGraph::AssignNode>("assign", FlowGraph::TypeInfo(FlowGraph::ValueType::Number), "x", "10");
        auto proc = std::make_shared<FlowGraph::ProcNode>("calc", "increment");
        auto cond = std::make_shared<FlowGraph::CondNode>("check", "x < 20");
        
        ast.addNode(assign);
        ast.addNode(proc); 
        ast.addNode(cond);
        
        // Add connections
        ast.addConnection(FlowGraph::FlowConnection("assign", "calc"));
        ast.addConnection(FlowGraph::FlowConnection("calc", "check"));
        
        std::cout << "✓ Flow AST created with " << ast.getNodes().size() << " nodes" << std::endl;
        
        std::cout << "\n🎉 FlowGraph find_package integration successful!" << std::endl;
        std::cout << "This example demonstrates using FlowGraph via CMake find_package." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}