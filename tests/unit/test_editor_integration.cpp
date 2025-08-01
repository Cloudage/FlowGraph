#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

// Include FlowGraph layout library headers
#include "../../include/flowgraph_layout/LayoutTypes.hpp"
#include "../../include/flowgraph_layout/Layout.hpp"
#include "../../include/flowgraph_layout/HierarchicalLayout.hpp"
#include "../../include/flowgraph_layout/ForceDirectedLayout.hpp"
#include "../../include/flowgraph_layout/GridLayout.hpp"

using namespace flowgraph::layout;
using namespace Catch::literals;

TEST_CASE("EditorApp - Graph layout integration", "[editor][layout][integration]") {
    // Note: This test verifies the integration without actually initializing graphics
    
    SECTION("Graph data structure creation") {
        // Create a graph similar to what EditorApp creates
        GraphF graph;
        
        // Add nodes similar to demo graph
        graph.addNode(NodeF(1, {100, 50}, {80, 40}));
        graph.addNode(NodeF(2, {50, 150}, {80, 40}));
        graph.addNode(NodeF(3, {150, 150}, {80, 40}));
        
        // Add edges
        graph.addEdge({1, 2});
        graph.addEdge({1, 3});
        
        REQUIRE(graph.nodeCount() == 3);
        REQUIRE(graph.edgeCount() == 2);
        
        // Verify nodes are accessible
        auto nodes = graph.getNodes();
        REQUIRE(nodes.find(1) != nodes.end());
        REQUIRE(nodes.find(2) != nodes.end());
        REQUIRE(nodes.find(3) != nodes.end());
        
        // Verify edges are accessible
        auto edges = graph.getEdges();
        REQUIRE(edges.size() == 2);
    }
    
    SECTION("Layout algorithms application") {
        GraphF graph;
        
        // Create a more complex demo graph
        for (size_t i = 1; i <= 9; ++i) {
            graph.addNode(NodeF(i, {static_cast<double>(i * 50), static_cast<double>(i * 30)}, {80, 40}));
        }
        
        // Add hierarchical structure
        graph.addEdge({1, 2});
        graph.addEdge({1, 3});
        graph.addEdge({2, 4});
        graph.addEdge({2, 5});
        graph.addEdge({3, 6});
        graph.addEdge({3, 7});
        graph.addEdge({1, 8});
        graph.addEdge({8, 9});
        
        REQUIRE(graph.nodeCount() == 9);
        REQUIRE(graph.edgeCount() == 8);
        
        // Test hierarchical layout
        {
            HierarchicalLayout<double> layout;
            LayoutConfig config;
            config.nodeSpacing = 60.0;
            config.layerSpacing = 80.0;
            
            auto result = layout.apply(graph, config);
            REQUIRE(result.success);
            
            // Verify nodes have been positioned
            auto nodes = graph.getNodes();
            for (const auto& pair : nodes) {
                const auto& node = pair.second;
                // Positions should be reasonable (not extreme values)
                REQUIRE(std::abs(node.position.x) < 10000);
                REQUIRE(std::abs(node.position.y) < 10000);
            }
        }
        
        // Test force-directed layout
        {
            ForceDirectedLayout<double> layout;
            LayoutConfig config;
            config.iterations = 50; // Limit iterations for test speed
            
            auto result = layout.apply(graph, config);
            REQUIRE(result.success);
            
            // Verify no overlaps (or very few)
            auto overlaps = utils::countOverlaps(graph);
            REQUIRE(overlaps <= 1); // Allow at most 1 overlap for small graph
        }
        
        // Test grid layout
        {
            GridLayout<double> layout;
            auto result = layout.apply(graph);
            REQUIRE(result.success);
            
            // Grid layout should produce zero overlaps
            auto overlaps = utils::countOverlaps(graph);
            REQUIRE(overlaps == 0);
        }
        
        // Test circular layout
        {
            CircularLayout<double> layout;
            auto result = layout.apply(graph);
            REQUIRE(result.success);
            
            // Circular layout should produce minimal overlaps for small graphs
            auto overlaps = utils::countOverlaps(graph);
            REQUIRE(overlaps <= 5); // Allow some overlaps for circular layout with many nodes
        }
    }
    
    SECTION("Graph visualization bounds calculation") {
        GraphF graph;
        
        // Add nodes at known positions
        graph.addNode(NodeF(1, {10, 20}, {50, 30}));
        graph.addNode(NodeF(2, {100, 200}, {50, 30}));
        graph.addNode(NodeF(3, {300, 50}, {50, 30}));
        
        auto nodes = graph.getNodes();
        
        // Calculate bounds similar to EditorApp::RenderGraph
        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        
        for (const auto& pair : nodes) {
            const auto& node = pair.second;
            min_x = std::min(min_x, node.position.x);
            max_x = std::max(max_x, node.position.x + node.size.x);
            min_y = std::min(min_y, node.position.y);
            max_y = std::max(max_y, node.position.y + node.size.y);
        }
        
        REQUIRE(min_x == 10.0_a);
        REQUIRE(max_x == 350.0_a); // 300 + 50
        REQUIRE(min_y == 20.0_a);
        REQUIRE(max_y == 230.0_a); // 200 + 30
        
        double graph_width = max_x - min_x;
        double graph_height = max_y - min_y;
        
        REQUIRE(graph_width == 340.0_a);
        REQUIRE(graph_height == 210.0_a);
    }
}

TEST_CASE("EditorApp - Layout algorithm switching", "[editor][layout]") {
    SECTION("Algorithm availability") {
        std::vector<std::string> available_layouts = {"hierarchical", "force_directed", "grid", "circular"};
        
        // Verify all algorithms can be instantiated
        for (const auto& algorithm_name : available_layouts) {
            std::unique_ptr<LayoutAlgorithm<double>> layout;
            
            if (algorithm_name == "hierarchical") {
                layout = std::make_unique<HierarchicalLayout<double>>();
            } else if (algorithm_name == "force_directed") {
                layout = std::make_unique<ForceDirectedLayout<double>>();
            } else if (algorithm_name == "grid") {
                layout = std::make_unique<GridLayout<double>>();
            } else if (algorithm_name == "circular") {
                layout = std::make_unique<CircularLayout<double>>();
            }
            
            REQUIRE(layout != nullptr);
            REQUIRE(layout->getName() == algorithm_name);
        }
    }
    
    SECTION("Layout switching behavior") {
        GraphF graph;
        
        // Create a simple test graph
        graph.addNode(NodeF(1, {0, 0}, {50, 30}));
        graph.addNode(NodeF(2, {100, 0}, {50, 30}));
        graph.addNode(NodeF(3, {200, 0}, {50, 30}));
        graph.addEdge({1, 2});
        graph.addEdge({2, 3});
        
        // Store initial positions
        auto initial_nodes = graph.getNodes();
        std::vector<PointF> initial_positions;
        for (const auto& pair : initial_nodes) {
            initial_positions.push_back(pair.second.position);
        }
        
        // Apply hierarchical layout
        HierarchicalLayout<double> hierarchical;
        auto result1 = hierarchical.apply(graph);
        REQUIRE(result1.success);
        
        auto hierarchical_nodes = graph.getNodes();
        std::vector<PointF> hierarchical_positions;
        for (const auto& pair : hierarchical_nodes) {
            hierarchical_positions.push_back(pair.second.position);
        }
        
        // Apply grid layout
        GridLayout<double> grid;
        auto result2 = grid.apply(graph);
        REQUIRE(result2.success);
        
        auto grid_nodes = graph.getNodes();
        std::vector<PointF> grid_positions;
        for (const auto& pair : grid_nodes) {
            grid_positions.push_back(pair.second.position);
        }
        
        // Positions should be different after different layout algorithms
        bool positions_changed = false;
        for (size_t i = 0; i < hierarchical_positions.size(); ++i) {
            if (hierarchical_positions[i].x != grid_positions[i].x ||
                hierarchical_positions[i].y != grid_positions[i].y) {
                positions_changed = true;
                break;
            }
        }
        
        REQUIRE(positions_changed); // Different algorithms should produce different layouts
    }
}