#include "flowgraph/FlowGraph.hpp"
#include <iostream>

int main() {
    try {
        std::cout << "FlowGraph Hello World Example\n";
        std::cout << "=============================\n\n";
        
        // TODO: Once the library is fully implemented, this will execute hello.flow
        std::cout << "FlowGraph library is ready for development!\n";
        std::cout << "This example will load and execute hello.flow when the parser and engine are complete.\n";
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}