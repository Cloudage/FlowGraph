#include <iostream>
#include "flowgraph/FlowGraph.hpp"

int main() {
    std::cout << "=== FlowGraph Standalone Integration Example ===" << std::endl;
    
    try {
        // Create FlowGraph engine
        FlowGraph::FlowGraphEngine engine;
        std::cout << "✓ FlowGraph engine created successfully" << std::endl;
        
        // Test ExpressionKit integration
        ExpressionKit::Value testValue(42.5);
        std::cout << "✓ ExpressionKit integration working: " << testValue.asString() << std::endl;
        
        // Create some basic AST nodes to verify functionality
        FlowGraph::AssignNode assignNode("start", FlowGraph::TypeInfo(FlowGraph::ValueType::Integer), "x", "0");
        FlowGraph::ProcNode procNode("process", "increment");
        
        std::cout << "✓ AST nodes created successfully" << std::endl;
        std::cout << "✓ Assign node: " << assignNode.id << std::endl;
        std::cout << "✓ Proc node: " << procNode.id << std::endl;
        
        std::cout << "\n🎉 FlowGraph standalone integration successful!" << std::endl;
        std::cout << "This example demonstrates using FlowGraph with manually copied headers." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}