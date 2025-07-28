#pragma once

#include <vector>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <memory>

namespace flowgraph::layout {

/// Basic 2D point structure
template<typename T = double>
struct Point {
    T x, y;
    
    Point() : x(0), y(0) {}
    Point(T x_, T y_) : x(x_), y(y_) {}
    
    Point operator+(const Point& other) const {
        return {x + other.x, y + other.y};
    }
    
    Point operator-(const Point& other) const {
        return {x - other.x, y - other.y};
    }
    
    Point operator*(T scalar) const {
        return {x * scalar, y * scalar};
    }
    
    T distanceTo(const Point& other) const {
        T dx = x - other.x;
        T dy = y - other.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    T magnitude() const {
        return std::sqrt(x * x + y * y);
    }
    
    Point normalized() const {
        T mag = magnitude();
        if (mag == 0) return {0, 0};
        return {x / mag, y / mag};
    }
};

using PointF = Point<double>;
using PointI = Point<int>;

/// Node identifier type
using NodeId = size_t;

/// Edge structure representing a connection between two nodes
struct Edge {
    NodeId from;
    NodeId to;
    
    Edge(NodeId f, NodeId t) : from(f), to(t) {}
    
    bool operator==(const Edge& other) const {
        return from == other.from && to == other.to;
    }
};

/// Node structure containing layout information
template<typename T = double>
struct Node {
    NodeId id;
    Point<T> position;
    Point<T> size;
    
    Node() : id(0), position{0, 0}, size{50, 30} {}
    Node(NodeId id_) : id(id_), position{0, 0}, size{50, 30} {}
    Node(NodeId id_, Point<T> pos) : id(id_), position(pos), size{50, 30} {}
    Node(NodeId id_, Point<T> pos, Point<T> sz) : id(id_), position(pos), size(sz) {}
    
    Point<T> center() const {
        return {position.x + size.x / 2, position.y + size.y / 2};
    }
    
    bool contains(const Point<T>& point) const {
        return point.x >= position.x && point.x <= position.x + size.x &&
               point.y >= position.y && point.y <= position.y + size.y;
    }
};

using NodeF = Node<double>;
using NodeI = Node<int>;

/// Graph structure for layout algorithms
template<typename T = double>
class Graph {
public:
    using NodeType = Node<T>;
    using PointType = Point<T>;
    
private:
    std::unordered_map<NodeId, NodeType> nodes_;
    std::vector<Edge> edges_;
    std::unordered_map<NodeId, std::vector<NodeId>> adjacency_list_;
    
public:
    /// Add a node to the graph
    void addNode(const NodeType& node) {
        nodes_[node.id] = node;
        if (adjacency_list_.find(node.id) == adjacency_list_.end()) {
            adjacency_list_[node.id] = {};
        }
    }
    
    /// Add an edge to the graph
    void addEdge(const Edge& edge) {
        edges_.push_back(edge);
        adjacency_list_[edge.from].push_back(edge.to);
        
        // Ensure both nodes exist in adjacency list
        if (adjacency_list_.find(edge.to) == adjacency_list_.end()) {
            adjacency_list_[edge.to] = {};
        }
    }
    
    /// Get node by ID
    NodeType* getNode(NodeId id) {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }
    
    const NodeType* getNode(NodeId id) const {
        auto it = nodes_.find(id);
        return it != nodes_.end() ? &it->second : nullptr;
    }
    
    /// Get all nodes
    const std::unordered_map<NodeId, NodeType>& getNodes() const { return nodes_; }
    
    /// Get all edges
    const std::vector<Edge>& getEdges() const { return edges_; }
    
    /// Get neighbors of a node
    const std::vector<NodeId>& getNeighbors(NodeId id) const {
        static const std::vector<NodeId> empty;
        auto it = adjacency_list_.find(id);
        return it != adjacency_list_.end() ? it->second : empty;
    }
    
    /// Get number of nodes
    size_t nodeCount() const { return nodes_.size(); }
    
    /// Get number of edges
    size_t edgeCount() const { return edges_.size(); }
    
    /// Clear all nodes and edges
    void clear() {
        nodes_.clear();
        edges_.clear();
        adjacency_list_.clear();
    }
    
    /// Update node position
    void updateNodePosition(NodeId id, const PointType& position) {
        auto it = nodes_.find(id);
        if (it != nodes_.end()) {
            it->second.position = position;
        }
    }
};

using GraphF = Graph<double>;
using GraphI = Graph<int>;

/// Layout configuration parameters
struct LayoutConfig {
    double nodeSpacing = 80.0;      ///< Minimum distance between nodes
    double layerSpacing = 100.0;    ///< Distance between layers (hierarchical)
    double iterations = 100;        ///< Number of iterations for iterative algorithms
    double convergenceThreshold = 1.0;  ///< Threshold for algorithm convergence
    bool preserveAspectRatio = true;     ///< Preserve aspect ratio during layout
    double marginX = 50.0;          ///< Horizontal margin
    double marginY = 50.0;          ///< Vertical margin
};

/// Layout result information
struct LayoutResult {
    bool success = false;
    size_t iterations = 0;
    double finalEnergy = 0.0;
    std::string errorMessage;
    PointF boundingBox;  ///< Total size of the layout
};

/// Base class for all layout algorithms
template<typename T = double>
class LayoutAlgorithm {
public:
    virtual ~LayoutAlgorithm() = default;
    
    /// Apply layout to the graph
    virtual LayoutResult apply(Graph<T>& graph, const LayoutConfig& config = {}) = 0;
    
    /// Get algorithm name
    virtual std::string getName() const = 0;
    
    /// Check if algorithm supports directed graphs
    virtual bool supportsDirectedGraphs() const { return true; }
    
    /// Check if algorithm is suitable for large graphs
    virtual bool isOptimizedForLargeGraphs() const { return false; }
};

} // namespace flowgraph::layout