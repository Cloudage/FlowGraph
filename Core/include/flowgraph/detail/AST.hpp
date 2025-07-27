#pragma once

#include "Types.hpp"
#include <memory>
#include <vector>

namespace FlowGraph {

/**
 * @brief Base class for all AST nodes
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    Location location;
    std::string comment;
    
protected:
    ASTNode(Location loc = {}) : location(loc) {}
};

/**
 * @brief Flow node base class
 */
class FlowNode : public ASTNode {
public:
    std::string id;
    
protected:
    FlowNode(const std::string& nodeId, Location loc = {}) 
        : ASTNode(loc), id(nodeId) {}
};

/**
 * @brief Assignment node (ASSIGN)
 */
class AssignNode : public FlowNode {
public:
    TypeInfo targetType;
    std::string variableName;
    std::string expression;
    
    AssignNode(const std::string& id, TypeInfo type, const std::string& var, 
               const std::string& expr, Location loc = {})
        : FlowNode(id, loc), targetType(type), variableName(var), expression(expr) {}
};

/**
 * @brief Condition node (COND)
 */
class CondNode : public FlowNode {
public:
    std::string condition;
    
    CondNode(const std::string& id, const std::string& cond, Location loc = {})
        : FlowNode(id, loc), condition(cond) {}
};

/**
 * @brief Procedure parameter binding
 */
struct ProcBinding {
    std::string localVar;
    std::string procParam;
    bool isOutput; // true for <<, false for >>
    
    ProcBinding(const std::string& local, const std::string& param, bool output)
        : localVar(local), procParam(param), isOutput(output) {}
};

/**
 * @brief Procedure call node (PROC)
 */
class ProcNode : public FlowNode {
public:
    std::string procedureName;
    std::vector<ProcBinding> bindings;
    
    ProcNode(const std::string& id, const std::string& proc, Location loc = {})
        : FlowNode(id, loc), procedureName(proc) {}
    
    void addBinding(const std::string& localVar, const std::string& procParam, bool isOutput) {
        bindings.emplace_back(localVar, procParam, isOutput);
    }
};

/**
 * @brief Flow connection
 */
struct FlowConnection {
    std::string fromNode;
    std::string toNode;
    std::string fromPort; // empty for default, "Y"/"N" for conditions
    std::string toPort;   // usually empty
    
    FlowConnection(const std::string& from, const std::string& to,
                   const std::string& fromP = "", const std::string& toP = "")
        : fromNode(from), toNode(to), fromPort(fromP), toPort(toP) {}
};

/**
 * @brief Error definition
 */
struct ErrorDefinition {
    std::string name;
    std::string comment;
    
    ErrorDefinition(const std::string& n, const std::string& c = "")
        : name(n), comment(c) {}
};

/**
 * @brief Complete FlowGraph AST
 */
class FlowAST : public ASTNode {
public:
    std::string title;
    std::vector<Parameter> parameters;
    std::vector<ReturnValue> returnValues;
    std::vector<ErrorDefinition> errors;
    std::vector<std::unique_ptr<FlowNode>> nodes;
    std::vector<FlowConnection> connections;
    
    FlowAST() = default;
    
    // Helper methods
    FlowNode* findNode(const std::string& id) const;
    std::vector<FlowConnection> getConnectionsFrom(const std::string& nodeId) const;
    std::vector<FlowConnection> getConnectionsTo(const std::string& nodeId) const;
    bool hasStartConnection() const;
    bool hasEndConnection() const;
    bool hasError(const std::string& errorName) const;
    
    // Validation
    std::vector<std::string> validate() const;
};

// Implementation (header-only)

inline FlowNode* FlowAST::findNode(const std::string& id) const {
    for (const auto& node : nodes) {
        if (node->id == id) {
            return node.get();
        }
    }
    return nullptr;
}

inline std::vector<FlowConnection> FlowAST::getConnectionsFrom(const std::string& nodeId) const {
    std::vector<FlowConnection> result;
    for (const auto& conn : connections) {
        if (conn.fromNode == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

inline std::vector<FlowConnection> FlowAST::getConnectionsTo(const std::string& nodeId) const {
    std::vector<FlowConnection> result;
    for (const auto& conn : connections) {
        if (conn.toNode == nodeId) {
            result.push_back(conn);
        }
    }
    return result;
}

inline bool FlowAST::hasStartConnection() const {
    for (const auto& conn : connections) {
        if (conn.fromNode == "START") {
            return true;
        }
    }
    return false;
}

inline bool FlowAST::hasEndConnection() const {
    for (const auto& conn : connections) {
        if (conn.toNode == "END") {
            return true;
        }
    }
    return false;
}

inline bool FlowAST::hasError(const std::string& errorName) const {
    for (const auto& error : errors) {
        if (error.name == errorName) {
            return true;
        }
    }
    return false;
}

inline std::vector<std::string> FlowAST::validate() const {
    std::vector<std::string> errors;
    
    // Check for START connection
    if (!hasStartConnection()) {
        errors.push_back("Flow must have a START connection");
    }
    
    // Check for END connection
    if (!hasEndConnection()) {
        errors.push_back("Flow must have at least one END connection");
    }
    
    // Check that all referenced nodes exist
    for (const auto& conn : connections) {
        if (conn.fromNode != "START" && !findNode(conn.fromNode)) {
            errors.push_back("Connection references unknown node: " + conn.fromNode);
        }
        if (conn.toNode != "END" && !findNode(conn.toNode)) {
            errors.push_back("Connection references unknown node: " + conn.toNode);
        }
    }
    
    return errors;
}

} // namespace FlowGraph