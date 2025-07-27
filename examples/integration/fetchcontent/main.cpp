#include <iostream>
#include <flowgraph/FlowGraph.hpp>

int main() {
    std::cout << "=== FlowGraph FetchContent Integration Example ===" << std::endl;
    
    try {
        // Create FlowGraph engine
        FlowGraph::FlowGraphEngine engine;
        std::cout << "✓ FlowGraph engine created successfully" << std::endl;
        
        // Test ExpressionKit integration
        ExpressionKit::Value testValue("Hello FetchContent!");
        std::cout << "✓ ExpressionKit integration working: " << testValue.asString() << std::endl;
        
        // Test mathematical expressions
        FlowGraph::FlowAST ast;
        FlowGraph::ExecutionContext context(ast);
        
        auto result = context.evaluateExpression("2 + 3 * 4");
        std::cout << "✓ Expression evaluation: 2 + 3 * 4 = " << result.asString() << std::endl;
        
        // Create some basic AST nodes
        FlowGraph::AssignNode assignNode("assign", FlowGraph::TypeInfo(FlowGraph::ValueType::Integer), "x", "0");
        FlowGraph::ProcNode procNode("process", "increment");
        FlowGraph::CondNode condNode("check", "x < 10");
        
        std::cout << "✓ AST nodes created successfully" << std::endl;
        std::cout << "✓ Process node procedure: " << procNode.procedureName << std::endl;
        
        std::cout << "\n🎉 FlowGraph FetchContent integration successful!" << std::endl;
        std::cout << "This example demonstrates using FlowGraph via CMake FetchContent." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}