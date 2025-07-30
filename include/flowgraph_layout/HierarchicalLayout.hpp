#pragma once

#include "LayoutTypes.hpp"
#include <queue>
#include <set>
#include <algorithm>
#include <numeric>

namespace flowgraph::layout {

/// Hierarchical layout algorithm based on Sugiyama framework
/// Suitable for directed acyclic graphs (DAGs) and trees
template<typename T = double>
class HierarchicalLayout : public LayoutAlgorithm<T> {
private:
    struct LayerInfo {
        std::vector<NodeId> nodes;
        T y_position = 0;
    };
    
    std::vector<LayerInfo> layers_;
    std::unordered_map<NodeId, size_t> node_to_layer_;
    
public:
    std::string getName() const override {
        return "hierarchical";
    }
    
    bool supportsDirectedGraphs() const override {
        return true;
    }
    
    LayoutResult apply(Graph<T>& graph, const LayoutConfig& config = {}) override {
        LayoutResult result;
        
        if (graph.nodeCount() == 0) {
            result.success = true;
            return result;
        }
        
        try {
            // Phase 1: Assign nodes to layers
            if (!assignLayers(graph)) {
                result.success = false;
                result.errorMessage = "Graph contains cycles - not suitable for hierarchical layout";
                return result;
            }
            
            // Phase 2: Reduce edge crossings
            reduceCrossings(graph, config);
            
            // Phase 3: Assign coordinates
            assignCoordinates(graph, config);
            
            result.success = true;
            result.boundingBox = calculateBoundingBox(graph);
            
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = e.what();
        }
        
        return result;
    }
    
private:
    /// Phase 1: Assign nodes to layers using longest path algorithm
    bool assignLayers(Graph<T>& graph) {
        layers_.clear();
        node_to_layer_.clear();
        
        // Calculate in-degree for each node
        std::unordered_map<NodeId, size_t> in_degree;
        for (const auto& pair : graph.getNodes()) {
            in_degree[pair.first] = 0;
        }
        
        for (const auto& edge : graph.getEdges()) {
            in_degree[edge.to]++;
        }
        
        // Topological sort with layer assignment
        std::queue<NodeId> ready_nodes;
        std::unordered_map<NodeId, size_t> node_layers;
        
        // Start with nodes having no incoming edges
        for (const auto& pair : in_degree) {
            if (pair.second == 0) {
                ready_nodes.push(pair.first);
                node_layers[pair.first] = 0;
            }
        }
        
        size_t processed_nodes = 0;
        size_t max_layer = 0;
        
        while (!ready_nodes.empty()) {
            NodeId current = ready_nodes.front();
            ready_nodes.pop();
            processed_nodes++;
            
            size_t current_layer = node_layers[current];
            max_layer = std::max(max_layer, current_layer);
            
            // Process all outgoing edges
            for (const auto& edge : graph.getEdges()) {
                if (edge.from == current) {
                    in_degree[edge.to]--;
                    
                    // Update layer assignment
                    node_layers[edge.to] = std::max(node_layers[edge.to], current_layer + 1);
                    
                    if (in_degree[edge.to] == 0) {
                        ready_nodes.push(edge.to);
                    }
                }
            }
        }
        
        // Check for cycles
        if (processed_nodes != graph.nodeCount()) {
            return false; // Graph contains cycles
        }
        
        // Organize nodes into layers
        layers_.resize(max_layer + 1);
        for (const auto& pair : node_layers) {
            size_t layer = pair.second;
            layers_[layer].nodes.push_back(pair.first);
            node_to_layer_[pair.first] = layer;
        }
        
        return true;
    }
    
    /// Phase 2: Reduce edge crossings using barycenter heuristic
    void reduceCrossings(Graph<T>& graph, const LayoutConfig& config) {
        const size_t max_iterations = static_cast<size_t>(config.iterations / 4); // Use 1/4 of total iterations
        
        for (size_t iter = 0; iter < max_iterations; ++iter) {
            bool changed = false;
            
            // Forward pass: fix upper layers, optimize lower layers
            for (size_t layer = 1; layer < layers_.size(); ++layer) {
                if (reorderLayer(graph, layer, true)) {
                    changed = true;
                }
            }
            
            // Backward pass: fix lower layers, optimize upper layers
            for (size_t layer = layers_.size() - 2; layer != SIZE_MAX; --layer) {
                if (reorderLayer(graph, layer, false)) {
                    changed = true;
                }
            }
            
            if (!changed) break; // Converged
        }
    }
    
    /// Reorder nodes in a layer to reduce crossings
    bool reorderLayer(Graph<T>& graph, size_t layer_idx, bool forward) {
        if (layer_idx >= layers_.size()) return false;
        
        auto& layer = layers_[layer_idx];
        if (layer.nodes.size() <= 1) return false;
        
        // Calculate barycenter for each node
        std::vector<std::pair<T, NodeId>> barycenters;
        
        for (NodeId node_id : layer.nodes) {
            T barycenter = calculateBarycenter(graph, node_id, forward);
            barycenters.emplace_back(barycenter, node_id);
        }
        
        // Sort by barycenter
        auto original_order = layer.nodes;
        std::sort(barycenters.begin(), barycenters.end());
        
        layer.nodes.clear();
        for (const auto& pair : barycenters) {
            layer.nodes.push_back(pair.second);
        }
        
        return original_order != layer.nodes;
    }
    
    /// Calculate barycenter position for a node
    T calculateBarycenter(Graph<T>& graph, NodeId node_id, bool forward) {
        size_t current_layer = node_to_layer_[node_id];
        T sum_positions = 0;
        size_t count = 0;
        
        if (forward && current_layer > 0) {
            // Look at predecessors in previous layer
            size_t prev_layer = current_layer - 1;
            for (const auto& edge : graph.getEdges()) {
                if (edge.to == node_id && node_to_layer_[edge.from] == prev_layer) {
                    // Find position of predecessor in its layer
                    const auto& prev_nodes = layers_[prev_layer].nodes;
                    auto it = std::find(prev_nodes.begin(), prev_nodes.end(), edge.from);
                    if (it != prev_nodes.end()) {
                        sum_positions += static_cast<T>(std::distance(prev_nodes.begin(), it));
                        count++;
                    }
                }
            }
        } else if (!forward && current_layer + 1 < layers_.size()) {
            // Look at successors in next layer
            size_t next_layer = current_layer + 1;
            for (const auto& edge : graph.getEdges()) {
                if (edge.from == node_id && node_to_layer_[edge.to] == next_layer) {
                    // Find position of successor in its layer
                    const auto& next_nodes = layers_[next_layer].nodes;
                    auto it = std::find(next_nodes.begin(), next_nodes.end(), edge.to);
                    if (it != next_nodes.end()) {
                        sum_positions += static_cast<T>(std::distance(next_nodes.begin(), it));
                        count++;
                    }
                }
            }
        }
        
        return count > 0 ? sum_positions / count : 0;
    }
    
    /// Phase 3: Assign final coordinates
    void assignCoordinates(Graph<T>& graph, const LayoutConfig& config) {
        if (layers_.empty()) return;
        
        T current_y = config.marginY;
        
        for (size_t layer_idx = 0; layer_idx < layers_.size(); ++layer_idx) {
            auto& layer = layers_[layer_idx];
            layer.y_position = current_y;
            
            // Calculate total width needed for this layer
            T total_node_width = 0;
            for (NodeId node_id : layer.nodes) {
                auto node = graph.getNode(node_id);
                if (node) {
                    total_node_width += node->size.x;
                }
            }
            
            T total_spacing = (layer.nodes.size() - 1) * config.nodeSpacing;
            T total_width = total_node_width + total_spacing;
            
            // Position nodes horizontally
            T current_x = config.marginX;
            if (total_width > 0) {
                // Center the layer if we have space
                current_x = config.marginX;
            }
            
            for (size_t i = 0; i < layer.nodes.size(); ++i) {
                NodeId node_id = layer.nodes[i];
                auto node = graph.getNode(node_id);
                if (node) {
                    graph.updateNodePosition(node_id, {current_x, current_y});
                    current_x += node->size.x + config.nodeSpacing;
                }
            }
            
            // Move to next layer
            current_y += findMaxNodeHeightInLayer(graph, layer_idx) + config.layerSpacing;
        }
    }
    
    /// Find maximum node height in a layer
    T findMaxNodeHeightInLayer(Graph<T>& graph, size_t layer_idx) {
        if (layer_idx >= layers_.size()) return 0;
        
        T max_height = 0;
        for (NodeId node_id : layers_[layer_idx].nodes) {
            auto node = graph.getNode(node_id);
            if (node) {
                max_height = std::max(max_height, node->size.y);
            }
        }
        return max_height > 0 ? max_height : 30; // Default height
    }
    
    /// Calculate bounding box of the layout
    Point<T> calculateBoundingBox(Graph<T>& graph) {
        T minX = std::numeric_limits<T>::max();
        T minY = std::numeric_limits<T>::max();
        T maxX = std::numeric_limits<T>::lowest();
        T maxY = std::numeric_limits<T>::lowest();
        
        for (const auto& pair : graph.getNodes()) {
            const auto& node = pair.second;
            minX = std::min(minX, node.position.x);
            minY = std::min(minY, node.position.y);
            maxX = std::max(maxX, node.position.x + node.size.x);
            maxY = std::max(maxY, node.position.y + node.size.y);
        }
        
        return {maxX - minX, maxY - minY};
    }
};

} // namespace flowgraph::layout