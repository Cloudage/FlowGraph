#include "flowgraph/FlowGraph.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "FlowGraph End-to-End Expression Demo\n";
        std::cout << "====================================\n\n";
        
        // Create a simple flow with expressions
        auto ast = std::make_unique<FlowGraph::FlowAST>();
        ast->title = "Calculator Flow";
        
        // Define parameters and return values
        ast->parameters.emplace_back("a", FlowGraph::TypeInfo(FlowGraph::ValueType::Float), "First number");
        ast->parameters.emplace_back("b", FlowGraph::TypeInfo(FlowGraph::ValueType::Float), "Second number");
        ast->returnValues.emplace_back("result", FlowGraph::TypeInfo(FlowGraph::ValueType::Float), "Calculation result");
        ast->returnValues.emplace_back("message", FlowGraph::TypeInfo(FlowGraph::ValueType::String), "Result message");
        
        // Create nodes that use expressions
        auto assign1 = std::make_unique<FlowGraph::AssignNode>("10", 
            FlowGraph::TypeInfo(FlowGraph::ValueType::Float), 
            "sum", 
            "a + b");
        
        auto assign2 = std::make_unique<FlowGraph::AssignNode>("20", 
            FlowGraph::TypeInfo(FlowGraph::ValueType::Float), 
            "product", 
            "a * b");
        
        auto assign3 = std::make_unique<FlowGraph::AssignNode>("30", 
            FlowGraph::TypeInfo(FlowGraph::ValueType::Float), 
            "result", 
            "max(sum, product)");
        
        auto assign4 = std::make_unique<FlowGraph::AssignNode>("40", 
            FlowGraph::TypeInfo(FlowGraph::ValueType::String), 
            "message", 
            "\"The maximum of (\" + a + \" + \" + b + \") and (\" + a + \" * \" + b + \") is \" + result");
        
        ast->nodes.push_back(std::move(assign1));
        ast->nodes.push_back(std::move(assign2));
        ast->nodes.push_back(std::move(assign3));
        ast->nodes.push_back(std::move(assign4));
        
        // Define flow connections
        ast->connections.emplace_back("START", "10");
        ast->connections.emplace_back("10", "20");
        ast->connections.emplace_back("20", "30");
        ast->connections.emplace_back("30", "40");
        ast->connections.emplace_back("40", "END");
        
        // Create and execute the flow
        FlowGraph::Flow flow(std::move(ast));
        
        // Test with some parameters
        FlowGraph::ParameterMap params;
        params["a"] = FlowGraph::createValue(5.0);
        params["b"] = FlowGraph::createValue(3.0);
        
        std::cout << "Input parameters:\n";
        std::cout << "a = " << params["a"].toString() << std::endl;
        std::cout << "b = " << params["b"].toString() << std::endl;
        std::cout << std::endl;
        
        // Execute the flow
        auto result = flow.execute(params);
        
        if (result.success) {
            std::cout << "Flow executed successfully!\n";
            std::cout << "Return values:\n";
            for (const auto& [key, value] : result.returnValues) {
                std::cout << key << " = " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Flow execution failed: " << result.error << std::endl;
        }
        
        std::cout << "\n✅ ExpressionKit integration allows complex expression evaluation in FlowGraph nodes!\n";
        std::cout << "Expressions are evaluated using ExpressionKit's powerful engine with full variable support.\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}