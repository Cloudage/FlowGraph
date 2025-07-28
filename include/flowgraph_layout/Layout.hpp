#pragma once

#include "LayoutTypes.hpp"
#include <memory>
#include <string>
#include <functional>
#include <stdexcept>

namespace flowgraph::layout {

/// Layout algorithm registry for managing different layout types
class LayoutRegistry {
private:
    using FactoryFunction = std::function<std::unique_ptr<LayoutAlgorithm<double>>()>;
    static std::unordered_map<std::string, FactoryFunction>& getFactories() {
        static std::unordered_map<std::string, FactoryFunction> factories;
        return factories;
    }
    
public:
    /// Register a layout algorithm
    template<typename AlgorithmType>
    static void registerAlgorithm(const std::string& name) {
        getFactories()[name] = []() -> std::unique_ptr<LayoutAlgorithm<double>> {
            return std::make_unique<AlgorithmType>();
        };
    }
    
    /// Create an instance of a layout algorithm
    static std::unique_ptr<LayoutAlgorithm<double>> create(const std::string& name) {
        auto& factories = getFactories();
        auto it = factories.find(name);
        if (it != factories.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    /// Get list of available algorithm names
    static std::vector<std::string> getAvailableAlgorithms() {
        std::vector<std::string> names;
        for (const auto& pair : getFactories()) {
            names.push_back(pair.first);
        }
        return names;
    }
};

/// High-level layout interface for easy use
class Layout {
private:
    std::unique_ptr<LayoutAlgorithm<double>> algorithm_;
    LayoutConfig config_;
    
public:
    /// Create layout with specified algorithm
    explicit Layout(const std::string& algorithmName) {
        algorithm_ = LayoutRegistry::create(algorithmName);
        if (!algorithm_) {
            throw std::runtime_error("Unknown layout algorithm: " + algorithmName);
        }
    }
    
    /// Create layout with algorithm instance
    explicit Layout(std::unique_ptr<LayoutAlgorithm<double>> algorithm)
        : algorithm_(std::move(algorithm)) {
        if (!algorithm_) {
            throw std::runtime_error("Layout algorithm cannot be null");
        }
    }
    
    /// Set configuration
    Layout& setConfig(const LayoutConfig& config) {
        config_ = config;
        return *this;
    }
    
    /// Get configuration
    const LayoutConfig& getConfig() const {
        return config_;
    }
    
    /// Apply layout to graph
    LayoutResult apply(GraphF& graph) {
        if (!algorithm_) {
            LayoutResult result;
            result.success = false;
            result.errorMessage = "No layout algorithm set";
            return result;
        }
        
        return algorithm_->apply(graph, config_);
    }
    
    /// Get algorithm name
    std::string getAlgorithmName() const {
        return algorithm_ ? algorithm_->getName() : "none";
    }
    
    /// Check if current algorithm supports directed graphs
    bool supportsDirectedGraphs() const {
        return algorithm_ ? algorithm_->supportsDirectedGraphs() : false;
    }
    
    /// Check if current algorithm is optimized for large graphs
    bool isOptimizedForLargeGraphs() const {
        return algorithm_ ? algorithm_->isOptimizedForLargeGraphs() : false;
    }
};

/// Utility functions for common layout operations
namespace utils {

/// Calculate bounding box of all nodes in the graph
template<typename T>
Point<T> calculateBoundingBox(const Graph<T>& graph) {
    if (graph.nodeCount() == 0) {
        return {0, 0};
    }
    
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

/// Center the graph around origin
template<typename T>
void centerGraph(Graph<T>& graph) {
    if (graph.nodeCount() == 0) return;
    
    T minX = std::numeric_limits<T>::max();
    T minY = std::numeric_limits<T>::max();
    T maxX = std::numeric_limits<T>::lowest();
    T maxY = std::numeric_limits<T>::lowest();
    
    // Find current bounds
    for (const auto& pair : graph.getNodes()) {
        const auto& node = pair.second;
        minX = std::min(minX, node.position.x);
        minY = std::min(minY, node.position.y);
        maxX = std::max(maxX, node.position.x + node.size.x);
        maxY = std::max(maxY, node.position.y + node.size.y);
    }
    
    // Calculate center offset
    T centerX = (maxX + minX) / 2;
    T centerY = (maxY + minY) / 2;
    
    // Move all nodes
    for (const auto& pair : graph.getNodes()) {
        NodeId id = pair.first;
        auto node = graph.getNode(id);
        if (node) {
            graph.updateNodePosition(id, {
                node->position.x - centerX,
                node->position.y - centerY
            });
        }
    }
}

/// Scale graph to fit within specified bounds
template<typename T>
void scaleToFit(Graph<T>& graph, T targetWidth, T targetHeight, T margin = 50) {
    if (graph.nodeCount() == 0) return;
    
    auto bounds = calculateBoundingBox(graph);
    if (bounds.x <= 0 || bounds.y <= 0) return;
    
    T availableWidth = targetWidth - 2 * margin;
    T availableHeight = targetHeight - 2 * margin;
    
    T scaleX = availableWidth / bounds.x;
    T scaleY = availableHeight / bounds.y;
    T scale = std::min(scaleX, scaleY);
    
    // Apply scaling
    for (const auto& pair : graph.getNodes()) {
        NodeId id = pair.first;
        auto node = graph.getNode(id);
        if (node) {
            graph.updateNodePosition(id, {
                node->position.x * scale + margin,
                node->position.y * scale + margin
            });
        }
    }
}

/// Check if two nodes overlap
template<typename T>
bool nodesOverlap(const Node<T>& a, const Node<T>& b, T padding = 0) {
    return !(a.position.x + a.size.x + padding <= b.position.x ||
             b.position.x + b.size.x + padding <= a.position.x ||
             a.position.y + a.size.y + padding <= b.position.y ||
             b.position.y + b.size.y + padding <= a.position.y);
}

/// Count overlapping nodes in the graph
template<typename T>
size_t countOverlaps(const Graph<T>& graph, T padding = 0) {
    size_t overlaps = 0;
    const auto& nodes = graph.getNodes();
    
    for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
        for (auto it2 = std::next(it1); it2 != nodes.end(); ++it2) {
            if (nodesOverlap(it1->second, it2->second, padding)) {
                overlaps++;
            }
        }
    }
    
    return overlaps;
}

/// Create a simple test graph for testing purposes
inline GraphF createTestGraph(size_t nodeCount, double edgeProbability = 0.3) {
    GraphF graph;
    
    // Add nodes
    for (size_t i = 0; i < nodeCount; ++i) {
        NodeF node(i);
        node.position = {
            static_cast<double>(rand() % 400),
            static_cast<double>(rand() % 400)
        };
        graph.addNode(node);
    }
    
    // Add random edges
    for (size_t i = 0; i < nodeCount; ++i) {
        for (size_t j = i + 1; j < nodeCount; ++j) {
            if (static_cast<double>(rand()) / RAND_MAX < edgeProbability) {
                graph.addEdge({i, j});
            }
        }
    }
    
    return graph;
}

} // namespace utils

} // namespace flowgraph::layout