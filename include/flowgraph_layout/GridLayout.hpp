#pragma once

#include "LayoutTypes.hpp"
#include <cmath>

namespace flowgraph::layout {

/// Simple grid-based layout algorithm
/// Arranges nodes in a regular grid pattern
template<typename T = double>
class GridLayout : public LayoutAlgorithm<T> {
public:
    std::string getName() const override {
        return "grid";
    }
    
    bool supportsDirectedGraphs() const override {
        return true; // Grid layout ignores edge direction
    }
    
    bool isOptimizedForLargeGraphs() const override {
        return true; // O(n) complexity
    }
    
    LayoutResult apply(Graph<T>& graph, const LayoutConfig& config = {}) override {
        LayoutResult result;
        
        if (graph.nodeCount() == 0) {
            result.success = true;
            return result;
        }
        
        try {
            size_t node_count = graph.nodeCount();
            
            // Calculate grid dimensions
            size_t grid_cols = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(node_count))));
            size_t grid_rows = static_cast<size_t>(std::ceil(static_cast<double>(node_count) / grid_cols));
            
            // Calculate cell size based on largest node
            T cell_width = findMaxNodeWidth(graph) + config.nodeSpacing;
            T cell_height = findMaxNodeHeight(graph) + config.nodeSpacing;
            
            // Ensure minimum cell size
            cell_width = std::max(cell_width, static_cast<T>(80));
            cell_height = std::max(cell_height, static_cast<T>(60));
            
            // Position nodes in grid
            size_t index = 0;
            for (const auto& pair : graph.getNodes()) {
                NodeId node_id = pair.first;
                
                size_t row = index / grid_cols;
                size_t col = index % grid_cols;
                
                T x = config.marginX + col * cell_width;
                T y = config.marginY + row * cell_height;
                
                // Center node within its cell
                auto node = graph.getNode(node_id);
                if (node) {
                    T center_x = x + (cell_width - node->size.x) / 2;
                    T center_y = y + (cell_height - node->size.y) / 2;
                    graph.updateNodePosition(node_id, {center_x, center_y});
                }
                
                index++;
            }
            
            result.success = true;
            result.boundingBox = {
                grid_cols * cell_width + 2 * config.marginX,
                grid_rows * cell_height + 2 * config.marginY
            };
            
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = e.what();
        }
        
        return result;
    }
    
private:
    /// Find maximum node width in the graph
    T findMaxNodeWidth(Graph<T>& graph) {
        T max_width = 0;
        for (const auto& pair : graph.getNodes()) {
            max_width = std::max(max_width, pair.second.size.x);
        }
        return max_width > 0 ? max_width : 50; // Default width
    }
    
    /// Find maximum node height in the graph
    T findMaxNodeHeight(Graph<T>& graph) {
        T max_height = 0;
        for (const auto& pair : graph.getNodes()) {
            max_height = std::max(max_height, pair.second.size.y);
        }
        return max_height > 0 ? max_height : 30; // Default height
    }
};

/// Circular layout algorithm
/// Arranges nodes in a circle
template<typename T = double>
class CircularLayout : public LayoutAlgorithm<T> {
public:
    std::string getName() const override {
        return "circular";
    }
    
    bool supportsDirectedGraphs() const override {
        return true; // Circular layout ignores edge direction
    }
    
    bool isOptimizedForLargeGraphs() const override {
        return true; // O(n) complexity
    }
    
    LayoutResult apply(Graph<T>& graph, const LayoutConfig& config = {}) override {
        LayoutResult result;
        
        if (graph.nodeCount() == 0) {
            result.success = true;
            return result;
        }
        
        try {
            size_t node_count = graph.nodeCount();
            
            if (node_count == 1) {
                // Single node at center
                auto& first_node = graph.getNodes().begin()->second;
                T center_x = config.marginX + 100;
                T center_y = config.marginY + 100;
                graph.updateNodePosition(first_node.id, {center_x, center_y});
                
                result.success = true;
                result.boundingBox = {200 + 2 * config.marginX, 200 + 2 * config.marginY};
                return result;
            }
            
            // Calculate radius based on number of nodes and desired spacing
            T circumference = node_count * config.nodeSpacing;
            T radius = circumference / (2 * M_PI);
            
            // Ensure minimum radius
            radius = std::max(radius, static_cast<T>(100));
            
            T center_x = config.marginX + radius;
            T center_y = config.marginY + radius;
            
            // Position nodes around circle
            size_t index = 0;
            for (const auto& pair : graph.getNodes()) {
                NodeId node_id = pair.first;
                
                T angle = 2 * M_PI * index / node_count;
                T x = center_x + radius * std::cos(angle);
                T y = center_y + radius * std::sin(angle);
                
                // Center node at calculated position
                auto node = graph.getNode(node_id);
                if (node) {
                    T node_x = x - node->size.x / 2;
                    T node_y = y - node->size.y / 2;
                    graph.updateNodePosition(node_id, {node_x, node_y});
                }
                
                index++;
            }
            
            T diameter = 2 * radius;
            result.success = true;
            result.boundingBox = {
                diameter + 2 * config.marginX,
                diameter + 2 * config.marginY
            };
            
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = e.what();
        }
        
        return result;
    }
};

} // namespace flowgraph::layout