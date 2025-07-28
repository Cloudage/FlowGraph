// Test that the layout library compiles independently of FlowGraph
// This file should compile with ONLY the layout library headers

#include "../include/flowgraph_layout/LayoutTypes.hpp"
#include "../include/flowgraph_layout/Layout.hpp"
#include "../include/flowgraph_layout/HierarchicalLayout.hpp"
#include "../include/flowgraph_layout/ForceDirectedLayout.hpp"
#include "../include/flowgraph_layout/GridLayout.hpp"

using namespace flowgraph::layout;

int main() {
    // Create a simple graph
    GraphF graph;
    graph.addNode(NodeF(1));
    graph.addNode(NodeF(2));
    graph.addNode(NodeF(3));
    graph.addEdge({1, 2});
    graph.addEdge({2, 3});
    
    // Test each layout algorithm
    GridLayout<double> grid;
    auto result1 = grid.apply(graph);
    
    CircularLayout<double> circular;
    auto result2 = circular.apply(graph);
    
    ForceDirectedLayout<double> force;
    auto result3 = force.apply(graph);
    
    HierarchicalLayout<double> hierarchical;
    auto result4 = hierarchical.apply(graph);
    
    // Test utility functions
    auto bbox = utils::calculateBoundingBox(graph);
    utils::centerGraph(graph);
    utils::scaleToFit(graph, 800.0, 600.0);
    size_t overlaps = utils::countOverlaps(graph);
    
    return (result1.success && result2.success && 
            result3.success && result4.success) ? 0 : 1;
}