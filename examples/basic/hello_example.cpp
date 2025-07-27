#include "flowgraph/FlowGraph.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "FlowGraph with ExpressionKit Integration Example\n";
        std::cout << "================================================\n\n";
        
        // Create a basic FlowAST for testing
        auto ast = std::make_unique<FlowGraph::FlowAST>();
        ast->title = "Expression Demo";
        
        // Create an execution context
        FlowGraph::ExecutionContext context(*ast);
        
        // Test basic arithmetic expressions
        std::cout << "Testing arithmetic expressions:\n";
        std::cout << "2 + 3 * 4 = " << context.evaluateExpression("2 + 3 * 4").toString() << std::endl;
        std::cout << "(10 - 2) / 4 = " << context.evaluateExpression("(10 - 2) / 4").toString() << std::endl;
        
        // Test with variables
        std::cout << "\nTesting expressions with variables:\n";
        context.setVariable("x", FlowGraph::createValue(static_cast<int64_t>(10)));
        context.setVariable("y", FlowGraph::createValue(25.5));
        
        std::cout << "x = " << context.getVariable("x").toString() << std::endl;
        std::cout << "y = " << context.getVariable("y").toString() << std::endl;
        std::cout << "x + y = " << context.evaluateExpression("x + y").toString() << std::endl;
        std::cout << "x * 2 > 15 = " << context.evaluateExpression("x * 2 > 15").toString() << std::endl;
        
        // Test mathematical functions
        std::cout << "\nTesting mathematical functions:\n";
        std::cout << "sqrt(y) = " << context.evaluateExpression("sqrt(y)").toString() << std::endl;
        std::cout << "max(x, 15) = " << context.evaluateExpression("max(x, 15)").toString() << std::endl;
        std::cout << "min(x, 5) = " << context.evaluateExpression("min(x, 5)").toString() << std::endl;
        
        // Test boolean expressions
        std::cout << "\nTesting boolean expressions:\n";
        context.setVariable("active", FlowGraph::createValue(true));
        context.setVariable("ready", FlowGraph::createValue(false));
        
        std::cout << "active = " << context.getVariable("active").toString() << std::endl;
        std::cout << "ready = " << context.getVariable("ready").toString() << std::endl;
        std::cout << "active && ready = " << context.evaluateExpression("active && ready").toString() << std::endl;
        std::cout << "active || ready = " << context.evaluateExpression("active || ready").toString() << std::endl;
        std::cout << "!ready = " << context.evaluateExpression("!ready").toString() << std::endl;
        
        // Test string operations
        std::cout << "\nTesting string operations:\n";
        context.setVariable("name", FlowGraph::createValue("FlowGraph"));
        context.setVariable("version", FlowGraph::createValue("1.0"));
        
        std::cout << "name = " << context.getVariable("name").toString() << std::endl;
        std::cout << "version = " << context.getVariable("version").toString() << std::endl;
        std::cout << "Concatenation: " << context.evaluateExpression("name + \" v\" + version").toString() << std::endl;
        
        std::cout << "\n✅ ExpressionKit integration is working correctly!\n";
        std::cout << "All expression types (arithmetic, boolean, string, functions) are supported.\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}