#pragma once

#include "LayoutTypes.hpp"
#include <random>
#include <cmath>

namespace flowgraph::layout {

/// Force-directed layout algorithm using Fruchterman-Reingold algorithm
/// Suitable for general graphs and provides natural-looking layouts
template<typename T = double>
class ForceDirectedLayout : public LayoutAlgorithm<T> {
private:
    struct ForceVector {
        T x, y;
        ForceVector(T x_ = 0, T y_ = 0) : x(x_), y(y_) {}
        
        ForceVector operator+(const ForceVector& other) const {
            return {x + other.x, y + other.y};
        }
        
        ForceVector operator*(T scalar) const {
            return {x * scalar, y * scalar};
        }
        
        T magnitude() const {
            return std::sqrt(x * x + y * y);
        }
        
        ForceVector normalized() const {
            T mag = magnitude();
            if (mag == 0) return {0, 0};
            return {x / mag, y / mag};
        }
    };
    
    std::mt19937 rng_;
    
public:
    ForceDirectedLayout() : rng_(std::random_device{}()) {}
    
    std::string getName() const override {
        return "force_directed";
    }
    
    bool supportsDirectedGraphs() const override {
        return true; // Works with both directed and undirected graphs
    }
    
    bool isOptimizedForLargeGraphs() const override {
        return false; // O(n²) complexity, not ideal for very large graphs
    }
    
    LayoutResult apply(Graph<T>& graph, const LayoutConfig& config = {}) override {
        LayoutResult result;
        
        if (graph.nodeCount() == 0) {
            result.success = true;
            return result;
        }
        
        try {
            // Initialize random positions if nodes don't have positions
            initializePositions(graph, config);
            
            // Calculate optimal edge length based on graph size and available space
            T optimal_edge_length = calculateOptimalEdgeLength(graph, config);
            
            // Run force-directed simulation
            size_t iterations = simulateForces(graph, config, optimal_edge_length);
            
            // Apply final adjustments
            removeOverlaps(graph, config);
            
            result.success = true;
            result.iterations = iterations;
            result.boundingBox = calculateBoundingBox(graph);
            
        } catch (const std::exception& e) {
            result.success = false;
            result.errorMessage = e.what();
        }
        
        return result;
    }
    
private:
    /// Initialize node positions randomly
    void initializePositions(Graph<T>& graph, const LayoutConfig& config) {
        std::uniform_real_distribution<T> dist_x(config.marginX, config.marginX + 400);
        std::uniform_real_distribution<T> dist_y(config.marginY, config.marginY + 400);
        
        for (const auto& pair : graph.getNodes()) {
            NodeId id = pair.first;
            auto node = graph.getNode(id);
            if (node) {
                // Only randomize if position is at origin (likely uninitialized)
                if (node->position.x == 0 && node->position.y == 0) {
                    graph.updateNodePosition(id, {dist_x(rng_), dist_y(rng_)});
                }
            }
        }
    }
    
    /// Calculate optimal edge length based on graph properties
    T calculateOptimalEdgeLength(Graph<T>& graph, const LayoutConfig& config) {
        size_t node_count = graph.nodeCount();
        if (node_count <= 1) return config.nodeSpacing;
        
        // Estimate area needed
        T area = node_count * config.nodeSpacing * config.nodeSpacing;
        T side_length = std::sqrt(area);
        
        // Optimal edge length should allow nodes to spread nicely
        return std::max(config.nodeSpacing, side_length / std::sqrt(static_cast<T>(node_count)));
    }
    
    /// Main force simulation loop
    size_t simulateForces(Graph<T>& graph, const LayoutConfig& config, T optimal_edge_length) {
        const size_t max_iterations = static_cast<size_t>(config.iterations);
        const T convergence_threshold = config.convergenceThreshold;
        
        // Temperature for simulated annealing
        T temperature = optimal_edge_length;
        const T cooling_factor = 0.95;
        const T min_temperature = 1.0;
        
        size_t iteration = 0;
        for (; iteration < max_iterations; ++iteration) {
            std::unordered_map<NodeId, ForceVector> forces;
            
            // Initialize forces
            for (const auto& pair : graph.getNodes()) {
                forces[pair.first] = ForceVector();
            }
            
            // Calculate repulsive forces between all pairs of nodes
            calculateRepulsiveForces(graph, forces, optimal_edge_length);
            
            // Calculate attractive forces for connected nodes
            calculateAttractiveForces(graph, forces, optimal_edge_length);
            
            // Apply forces and update positions
            T max_displacement = applyForces(graph, forces, temperature);
            
            // Cool down
            temperature = std::max(min_temperature, temperature * cooling_factor);
            
            // Check for convergence
            if (max_displacement < convergence_threshold) {
                break;
            }
        }
        
        return iteration;
    }
    
    /// Calculate repulsive forces between all pairs of nodes
    void calculateRepulsiveForces(Graph<T>& graph, 
                                 std::unordered_map<NodeId, ForceVector>& forces,
                                 T optimal_edge_length) {
        const auto& nodes = graph.getNodes();
        const T k_repulsive = optimal_edge_length * optimal_edge_length;
        
        for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
            for (auto it2 = std::next(it1); it2 != nodes.end(); ++it2) {
                const auto& node1 = it1->second;
                const auto& node2 = it2->second;
                
                Point<T> center1 = node1.center();
                Point<T> center2 = node2.center();
                
                Point<T> delta = center1 - center2;
                T distance = delta.magnitude();
                
                if (distance > 0 && distance < optimal_edge_length * 3) { // Limit range
                    T force_magnitude = k_repulsive / (distance * distance);
                    Point<T> force_direction = delta.normalized();
                    
                    ForceVector force(force_direction.x * force_magnitude, 
                                    force_direction.y * force_magnitude);
                    
                    forces[it1->first] = forces[it1->first] + force;
                    forces[it2->first] = forces[it2->first] + (force * -1);
                }
            }
        }
    }
    
    /// Calculate attractive forces for connected nodes
    void calculateAttractiveForces(Graph<T>& graph,
                                  std::unordered_map<NodeId, ForceVector>& forces,
                                  T optimal_edge_length) {
        for (const auto& edge : graph.getEdges()) {
            auto node1 = graph.getNode(edge.from);
            auto node2 = graph.getNode(edge.to);
            
            if (!node1 || !node2) continue;
            
            Point<T> center1 = node1->center();
            Point<T> center2 = node2->center();
            
            Point<T> delta = center2 - center1;
            T distance = delta.magnitude();
            
            if (distance > 0) {
                T force_magnitude = (distance * distance) / optimal_edge_length;
                Point<T> force_direction = delta.normalized();
                
                ForceVector force(force_direction.x * force_magnitude,
                                force_direction.y * force_magnitude);
                
                forces[edge.from] = forces[edge.from] + force;
                forces[edge.to] = forces[edge.to] + (force * -1);
            }
        }
    }
    
    /// Apply forces to nodes and update positions
    T applyForces(Graph<T>& graph,
                  const std::unordered_map<NodeId, ForceVector>& forces,
                  T temperature) {
        T max_displacement = 0;
        
        for (const auto& pair : forces) {
            NodeId node_id = pair.first;
            const ForceVector& force = pair.second;
            
            auto node = graph.getNode(node_id);
            if (!node) continue;
            
            // Limit displacement by temperature
            T displacement_magnitude = std::min(force.magnitude(), temperature);
            
            if (displacement_magnitude > 0) {
                ForceVector displacement = force.normalized() * displacement_magnitude;
                
                Point<T> new_position = {
                    node->position.x + displacement.x,
                    node->position.y + displacement.y
                };
                
                graph.updateNodePosition(node_id, new_position);
                max_displacement = std::max(max_displacement, displacement_magnitude);
            }
        }
        
        return max_displacement;
    }
    
    /// Remove overlaps between nodes using simple separation
    void removeOverlaps(Graph<T>& graph, const LayoutConfig& config) {
        const size_t max_overlap_iterations = 10;
        const T min_separation = config.nodeSpacing * 0.5;
        
        for (size_t iter = 0; iter < max_overlap_iterations; ++iter) {
            bool had_overlaps = false;
            const auto& nodes = graph.getNodes();
            
            for (auto it1 = nodes.begin(); it1 != nodes.end(); ++it1) {
                for (auto it2 = std::next(it1); it2 != nodes.end(); ++it2) {
                    const auto& node1 = it1->second;
                    const auto& node2 = it2->second;
                    
                    if (nodesOverlap(node1, node2, min_separation)) {
                        separateNodes(graph, it1->first, it2->first, min_separation);
                        had_overlaps = true;
                    }
                }
            }
            
            if (!had_overlaps) break;
        }
    }
    
    /// Check if two nodes overlap
    bool nodesOverlap(const Node<T>& a, const Node<T>& b, T padding) {
        return !(a.position.x + a.size.x + padding <= b.position.x ||
                 b.position.x + b.size.x + padding <= a.position.x ||
                 a.position.y + a.size.y + padding <= b.position.y ||
                 b.position.y + b.size.y + padding <= a.position.y);
    }
    
    /// Separate two overlapping nodes
    void separateNodes(Graph<T>& graph, NodeId id1, NodeId id2, T min_separation) {
        auto node1 = graph.getNode(id1);
        auto node2 = graph.getNode(id2);
        
        if (!node1 || !node2) return;
        
        Point<T> center1 = node1->center();
        Point<T> center2 = node2->center();
        
        Point<T> delta = center1 - center2;
        T distance = delta.magnitude();
        
        if (distance == 0) {
            // Nodes are at same position, move them apart randomly
            std::uniform_real_distribution<T> dist(-min_separation, min_separation);
            delta = {dist(rng_), dist(rng_)};
            distance = delta.magnitude();
        }
        
        if (distance > 0) {
            T required_distance = (node1->size.x + node2->size.x) / 2 + min_separation;
            T separation_distance = (required_distance - distance) / 2;
            
            Point<T> separation_vector = delta.normalized() * separation_distance;
            
            graph.updateNodePosition(id1, {
                node1->position.x + separation_vector.x,
                node1->position.y + separation_vector.y
            });
            
            graph.updateNodePosition(id2, {
                node2->position.x - separation_vector.x,
                node2->position.y - separation_vector.y
            });
        }
    }
    
    /// Calculate bounding box of the layout
    Point<T> calculateBoundingBox(Graph<T>& graph) {
        if (graph.nodeCount() == 0) return {0, 0};
        
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