#include <iostream>
#include <memory>
#include "../include/flowgraph_layout/LayoutTypes.hpp"
#include "../include/flowgraph_layout/Layout.hpp"
#include "../include/flowgraph_layout/HierarchicalLayout.hpp"
#include "../include/flowgraph_layout/ForceDirectedLayout.hpp"
#include "../include/flowgraph_layout/GridLayout.hpp"

using namespace flowgraph::layout;

void printGraph(const GraphF& graph, const std::string& title) {
    std::cout << "\n=== " << title << " ===\n";
    for (const auto& pair : graph.getNodes()) {
        const auto& node = pair.second;
        std::cout << "Node " << node.id << ": ("
                  << node.position.x << ", " << node.position.y << ")\n";
    }
    
    auto bbox = utils::calculateBoundingBox(graph);
    std::cout << "Bounding box: " << bbox.x << " x " << bbox.y << "\n";
    std::cout << "Overlaps: " << utils::countOverlaps(graph) << "\n";
}

void demonstrateLayoutAlgorithm(const std::string& algorithmName, 
                               std::unique_ptr<LayoutAlgorithm<double>> algorithm) {
    std::cout << "\n\n########## " << algorithmName << " Layout ##########\n";
    
    // Create a test graph
    GraphF graph;
    
    // Add nodes in a simple hierarchy
    graph.addNode(NodeF(1, {0, 0}));     // Root
    graph.addNode(NodeF(2, {0, 0}));     // Children
    graph.addNode(NodeF(3, {0, 0}));
    graph.addNode(NodeF(4, {0, 0}));     // Grandchildren
    graph.addNode(NodeF(5, {0, 0}));
    graph.addNode(NodeF(6, {0, 0}));
    
    // Add edges (tree structure)
    graph.addEdge({1, 2});
    graph.addEdge({1, 3});
    graph.addEdge({2, 4});
    graph.addEdge({2, 5});
    graph.addEdge({3, 6});
    
    // Additional connections for more interesting layouts
    if (algorithmName != "Hierarchical") {
        graph.addEdge({4, 6}); // Cross connection (creates cycle for hierarchical)
    }
    
    printGraph(graph, "Before Layout");
    
    // Apply layout
    LayoutConfig config;
    config.nodeSpacing = 80.0;
    config.layerSpacing = 120.0;
    config.iterations = 100;
    
    auto result = algorithm->apply(graph, config);
    
    std::cout << "\nLayout Result:\n";
    std::cout << "Success: " << (result.success ? "Yes" : "No") << "\n";
    if (!result.success) {
        std::cout << "Error: " << result.errorMessage << "\n";
        return;
    }
    
    std::cout << "Iterations: " << result.iterations << "\n";
    std::cout << "Final bounding box: " << result.boundingBox.x 
              << " x " << result.boundingBox.y << "\n";
    
    printGraph(graph, "After Layout");
}

int main() {
    std::cout << "FlowGraph Layout Library Demo\n";
    std::cout << "=============================\n";
    
    // Demonstrate each layout algorithm
    demonstrateLayoutAlgorithm("Grid", std::make_unique<GridLayout<double>>());
    demonstrateLayoutAlgorithm("Circular", std::make_unique<CircularLayout<double>>());
    demonstrateLayoutAlgorithm("Force-Directed", std::make_unique<ForceDirectedLayout<double>>());
    demonstrateLayoutAlgorithm("Hierarchical", std::make_unique<HierarchicalLayout<double>>());
    
    // Demonstrate utility functions
    std::cout << "\n\n########## Utility Functions ##########\n";
    
    // Create a larger test graph
    auto large_graph = utils::createTestGraph(20, 0.2);
    std::cout << "\nCreated test graph with " << large_graph.nodeCount() 
              << " nodes and " << large_graph.edgeCount() << " edges\n";
    
    // Apply grid layout
    GridLayout<double> grid_layout;
    LayoutConfig config;
    grid_layout.apply(large_graph, config);
    
    printGraph(large_graph, "Large Graph with Grid Layout");
    
    // Center the graph
    utils::centerGraph(large_graph);
    printGraph(large_graph, "After Centering");
    
    // Scale to fit in a 800x600 area
    utils::scaleToFit(large_graph, 800.0, 600.0, 50.0);
    printGraph(large_graph, "After Scaling to 800x600");
    
    std::cout << "\n\nDemo completed successfully!\n";
    std::cout << "The layout library is working independently of FlowGraph core.\n";
    
    return 0;
}