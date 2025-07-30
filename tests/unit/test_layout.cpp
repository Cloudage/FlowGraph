#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <chrono>

// Include all layout headers
#include "../../include/flowgraph_layout/LayoutTypes.hpp"
#include "../../include/flowgraph_layout/Layout.hpp"
#include "../../include/flowgraph_layout/HierarchicalLayout.hpp"
#include "../../include/flowgraph_layout/ForceDirectedLayout.hpp"
#include "../../include/flowgraph_layout/GridLayout.hpp"

using namespace flowgraph::layout;
using namespace Catch::literals;

TEST_CASE("LayoutTypes - Point operations", "[layout][types]") {
    SECTION("Point construction and basic operations") {
        PointF p1(3.0, 4.0);
        PointF p2(1.0, 2.0);
        
        REQUIRE(p1.x == 3.0);
        REQUIRE(p1.y == 4.0);
        
        auto p3 = p1 + p2;
        REQUIRE(p3.x == 4.0);
        REQUIRE(p3.y == 6.0);
        
        auto p4 = p1 - p2;
        REQUIRE(p4.x == 2.0);
        REQUIRE(p4.y == 2.0);
        
        auto p5 = p1 * 2.0;
        REQUIRE(p5.x == 6.0);
        REQUIRE(p5.y == 8.0);
    }
    
    SECTION("Point distance and magnitude") {
        PointF p1(0.0, 0.0);
        PointF p2(3.0, 4.0);
        
        REQUIRE(p1.distanceTo(p2) == 5.0_a);
        REQUIRE(p2.magnitude() == 5.0_a);
        
        auto normalized = p2.normalized();
        REQUIRE(normalized.magnitude() == 1.0_a);
        REQUIRE(normalized.x == 0.6_a);
        REQUIRE(normalized.y == 0.8_a);
    }
}

TEST_CASE("LayoutTypes - Node operations", "[layout][types]") {
    SECTION("Node construction and properties") {
        NodeF node(1, {10.0, 20.0}, {50.0, 30.0});
        
        REQUIRE(node.id == 1);
        REQUIRE(node.position.x == 10.0);
        REQUIRE(node.position.y == 20.0);
        REQUIRE(node.size.x == 50.0);
        REQUIRE(node.size.y == 30.0);
        
        auto center = node.center();
        REQUIRE(center.x == 35.0);
        REQUIRE(center.y == 35.0);
    }
    
    SECTION("Node contains point") {
        NodeF node(1, {10.0, 20.0}, {50.0, 30.0});
        
        REQUIRE(node.contains({15.0, 25.0}));
        REQUIRE(node.contains({10.0, 20.0})); // Corner
        REQUIRE(node.contains({60.0, 50.0})); // Corner
        REQUIRE_FALSE(node.contains({5.0, 15.0}));
        REQUIRE_FALSE(node.contains({65.0, 55.0}));
    }
}

TEST_CASE("LayoutTypes - Graph operations", "[layout][types]") {
    SECTION("Graph construction and basic operations") {
        GraphF graph;
        
        REQUIRE(graph.nodeCount() == 0);
        REQUIRE(graph.edgeCount() == 0);
        
        // Add nodes
        graph.addNode(NodeF(1, {0.0, 0.0}));
        graph.addNode(NodeF(2, {100.0, 0.0}));
        graph.addNode(NodeF(3, {50.0, 100.0}));
        
        REQUIRE(graph.nodeCount() == 3);
        
        // Add edges
        graph.addEdge({1, 2});
        graph.addEdge({2, 3});
        graph.addEdge({1, 3});
        
        REQUIRE(graph.edgeCount() == 3);
    }
    
    SECTION("Graph neighbor access") {
        GraphF graph;
        graph.addNode(NodeF(1));
        graph.addNode(NodeF(2));
        graph.addNode(NodeF(3));
        
        graph.addEdge({1, 2});
        graph.addEdge({1, 3});
        
        auto neighbors = graph.getNeighbors(1);
        REQUIRE(neighbors.size() == 2);
        REQUIRE(std::find(neighbors.begin(), neighbors.end(), 2) != neighbors.end());
        REQUIRE(std::find(neighbors.begin(), neighbors.end(), 3) != neighbors.end());
        
        REQUIRE(graph.getNeighbors(2).size() == 0);
        REQUIRE(graph.getNeighbors(4).size() == 0); // Non-existent node
    }
}

TEST_CASE("GridLayout - Basic functionality", "[layout][grid]") {
    SECTION("Empty graph") {
        GraphF graph;
        GridLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        REQUIRE(graph.nodeCount() == 0);
    }
    
    SECTION("Single node") {
        GraphF graph;
        graph.addNode(NodeF(1));
        
        GridLayout<double> layout;
        LayoutConfig config;
        config.marginX = 10.0;
        config.marginY = 10.0;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        auto node = graph.getNode(1);
        REQUIRE(node != nullptr);
        REQUIRE(node->position.x >= config.marginX);
        REQUIRE(node->position.y >= config.marginY);
    }
    
    SECTION("Multiple nodes arranged in grid") {
        GraphF graph;
        for (size_t i = 1; i <= 9; ++i) {
            graph.addNode(NodeF(i));
        }
        
        GridLayout<double> layout;
        LayoutConfig config;
        config.nodeSpacing = 20.0;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        REQUIRE(result.boundingBox.x > 0);
        REQUIRE(result.boundingBox.y > 0);
        
        // Check that nodes don't overlap
        const auto& nodes = graph.getNodes();
        for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != nodes.end(); ++it2) {
                REQUIRE_FALSE(utils::nodesOverlap(it1->second, it2->second));
            }
        }
    }
}

TEST_CASE("CircularLayout - Basic functionality", "[layout][circular]") {
    SECTION("Single node centered") {
        GraphF graph;
        graph.addNode(NodeF(1));
        
        CircularLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        auto node = graph.getNode(1);
        REQUIRE(node != nullptr);
        // Node should be positioned somewhere reasonable
        REQUIRE(node->position.x > 0);
        REQUIRE(node->position.y > 0);
    }
    
    SECTION("Multiple nodes in circle") {
        GraphF graph;
        for (size_t i = 1; i <= 6; ++i) {
            graph.addNode(NodeF(i));
        }
        
        CircularLayout<double> layout;
        LayoutConfig config;
        config.nodeSpacing = 50.0;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        // Verify nodes are arranged around a center point
        const auto& nodes = graph.getNodes();
        
        // Calculate center of all nodes
        double total_x = 0, total_y = 0;
        for (const auto& pair : nodes) {
            auto center = pair.second.center();
            total_x += center.x;
            total_y += center.y;
        }
        double center_x = total_x / nodes.size();
        double center_y = total_y / nodes.size();
        
        // All nodes should be roughly equidistant from center
        double first_distance = 0;
        bool first = true;
        for (const auto& pair : nodes) {
            auto node_center = pair.second.center();
            double distance = std::sqrt(
                (node_center.x - center_x) * (node_center.x - center_x) +
                (node_center.y - center_y) * (node_center.y - center_y)
            );
            
            if (first) {
                first_distance = distance;
                first = false;
            } else {
                REQUIRE(distance == Catch::Approx(first_distance).margin(10.0));
            }
        }
    }
}

TEST_CASE("ForceDirectedLayout - Basic functionality", "[layout][force]") {
    SECTION("Empty graph") {
        GraphF graph;
        ForceDirectedLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        REQUIRE(graph.nodeCount() == 0);
    }
    
    SECTION("Two connected nodes") {
        GraphF graph;
        graph.addNode(NodeF(1, {0.0, 0.0}));
        graph.addNode(NodeF(2, {0.0, 0.0})); // Start at same position
        graph.addEdge({1, 2});
        
        ForceDirectedLayout<double> layout;
        LayoutConfig config;
        config.iterations = 50;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        REQUIRE(result.iterations <= 50);
        
        // Nodes should be separated after layout
        auto node1 = graph.getNode(1);
        auto node2 = graph.getNode(2);
        REQUIRE(node1 != nullptr);
        REQUIRE(node2 != nullptr);
        
        double distance = node1->center().distanceTo(node2->center());
        REQUIRE(distance > 10.0); // Should be reasonably separated
    }
    
    SECTION("Triangle graph") {
        GraphF graph;
        graph.addNode(NodeF(1));
        graph.addNode(NodeF(2));
        graph.addNode(NodeF(3));
        
        graph.addEdge({1, 2});
        graph.addEdge({2, 3});
        graph.addEdge({3, 1});
        
        ForceDirectedLayout<double> layout;
        LayoutConfig config;
        config.iterations = 100;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        // Nodes should not overlap
        REQUIRE(utils::countOverlaps(graph) == 0);
    }
}

TEST_CASE("HierarchicalLayout - Basic functionality", "[layout][hierarchical]") {
    SECTION("Empty graph") {
        GraphF graph;
        HierarchicalLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        REQUIRE(graph.nodeCount() == 0);
    }
    
    SECTION("Linear chain (DAG)") {
        GraphF graph;
        graph.addNode(NodeF(1));
        graph.addNode(NodeF(2));
        graph.addNode(NodeF(3));
        graph.addNode(NodeF(4));
        
        graph.addEdge({1, 2});
        graph.addEdge({2, 3});
        graph.addEdge({3, 4});
        
        HierarchicalLayout<double> layout;
        LayoutConfig config;
        config.layerSpacing = 100.0;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        // Nodes should be arranged vertically in order
        auto node1 = graph.getNode(1);
        auto node2 = graph.getNode(2);
        auto node3 = graph.getNode(3);
        auto node4 = graph.getNode(4);
        
        REQUIRE(node1 != nullptr);
        REQUIRE(node2 != nullptr);
        REQUIRE(node3 != nullptr);
        REQUIRE(node4 != nullptr);
        
        // Check vertical ordering
        REQUIRE(node1->position.y < node2->position.y);
        REQUIRE(node2->position.y < node3->position.y);
        REQUIRE(node3->position.y < node4->position.y);
    }
    
    SECTION("Graph with cycle") {
        GraphF graph;
        graph.addNode(NodeF(1));
        graph.addNode(NodeF(2));
        graph.addNode(NodeF(3));
        
        graph.addEdge({1, 2});
        graph.addEdge({2, 3});
        graph.addEdge({3, 1}); // Creates cycle
        
        HierarchicalLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.errorMessage.find("cycle") != std::string::npos);
    }
    
    SECTION("Tree structure") {
        GraphF graph;
        // Root
        graph.addNode(NodeF(1));
        // Level 1
        graph.addNode(NodeF(2));
        graph.addNode(NodeF(3));
        // Level 2
        graph.addNode(NodeF(4));
        graph.addNode(NodeF(5));
        graph.addNode(NodeF(6));
        
        graph.addEdge({1, 2});
        graph.addEdge({1, 3});
        graph.addEdge({2, 4});
        graph.addEdge({2, 5});
        graph.addEdge({3, 6});
        
        HierarchicalLayout<double> layout;
        LayoutConfig config;
        
        auto result = layout.apply(graph, config);
        REQUIRE(result.success);
        
        // Root should be at top
        auto root = graph.getNode(1);
        auto child1 = graph.getNode(2);
        auto grandchild = graph.getNode(4);
        
        REQUIRE(root->position.y < child1->position.y);
        REQUIRE(child1->position.y < grandchild->position.y);
    }
}

TEST_CASE("Layout utility functions", "[layout][utils]") {
    SECTION("Bounding box calculation") {
        GraphF graph;
        graph.addNode(NodeF(1, {10.0, 20.0}, {50.0, 30.0}));
        graph.addNode(NodeF(2, {100.0, 50.0}, {50.0, 30.0}));
        
        auto bbox = utils::calculateBoundingBox(graph);
        REQUIRE(bbox.x == 140.0); // 150 - 10
        REQUIRE(bbox.y == 60.0);  // 80 - 20
    }
    
    SECTION("Center graph") {
        GraphF graph;
        graph.addNode(NodeF(1, {100.0, 100.0}));
        graph.addNode(NodeF(2, {200.0, 200.0}));
        
        utils::centerGraph(graph);
        
        auto node1 = graph.getNode(1);
        auto node2 = graph.getNode(2);
        
        // After centering, the midpoint should be near origin
        double mid_x = (node1->position.x + node2->position.x + node2->size.x) / 2;
        double mid_y = (node1->position.y + node2->position.y + node2->size.y) / 2;
        
        REQUIRE(std::abs(mid_x) < 50.0);
        REQUIRE(std::abs(mid_y) < 50.0);
    }
    
    SECTION("Overlap detection") {
        NodeF node1(1, {0.0, 0.0}, {50.0, 30.0});
        NodeF node2(2, {25.0, 15.0}, {50.0, 30.0}); // Overlapping
        NodeF node3(3, {100.0, 100.0}, {50.0, 30.0}); // Not overlapping
        
        REQUIRE(utils::nodesOverlap(node1, node2));
        REQUIRE_FALSE(utils::nodesOverlap(node1, node3));
        REQUIRE_FALSE(utils::nodesOverlap(node2, node3));
    }
    
    SECTION("Test graph creation") {
        auto graph = utils::createTestGraph(5, 0.5);
        
        REQUIRE(graph.nodeCount() == 5);
        REQUIRE(graph.edgeCount() > 0); // Should have some edges with 50% probability
        
        // All nodes should have reasonable positions
        for (const auto& pair : graph.getNodes()) {
            const auto& node = pair.second;
            REQUIRE(node.position.x >= 0);
            REQUIRE(node.position.y >= 0);
            REQUIRE(node.position.x < 400);
            REQUIRE(node.position.y < 400);
        }
    }
}

TEST_CASE("Performance test with large graph", "[layout][performance]") {
    SECTION("Grid layout with 100 nodes") {
        GraphF graph;
        for (size_t i = 1; i <= 100; ++i) {
            graph.addNode(NodeF(i));
        }
        
        GridLayout<double> layout;
        LayoutConfig config;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = layout.apply(graph, config);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        REQUIRE(result.success);
        REQUIRE(duration.count() < 1000); // Should complete in less than 1 second
        REQUIRE(utils::countOverlaps(graph) == 0); // No overlaps
    }
    
    SECTION("Force directed layout with 50 nodes") {
        auto graph = utils::createTestGraph(50, 0.1);
        
        ForceDirectedLayout<double> layout;
        LayoutConfig config;
        config.iterations = 50; // Limit iterations for performance
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = layout.apply(graph, config);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        REQUIRE(result.success);
        REQUIRE(duration.count() < 5000); // Should complete in less than 5 seconds
    }
}