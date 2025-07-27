#pragma once

#include "AST.hpp"
#include "Types.hpp"
#include <unordered_map>
#include <memory>

// Forward declare ExpressionKit types
namespace ExpressionKit {
    class Expression;
    class Context;
}

namespace FlowGraph {

/**
 * @brief Execution context for a single flow execution
 */
class ExecutionContext {
public:
    ExecutionContext(const FlowAST& ast);
    
    // Variable management
    void setVariable(const std::string& name, const Value& value);
    Value getVariable(const std::string& name) const;
    bool hasVariable(const std::string& name) const;
    
    // Parameter binding
    void bindParameters(const ParameterMap& params);
    ParameterMap extractReturnValues() const;
    
    // Expression evaluation
    Value evaluateExpression(const std::string& expression);
    
private:
    const FlowAST& ast_;
    ParameterMap variables_;
    std::unique_ptr<ExpressionKit::Context> exprContext_;
    
    void initializeExpressionContext();
    void updateExpressionContext();
};

/**
 * @brief Loaded and ready-to-execute flow
 */
class Flow {
public:
    Flow(std::unique_ptr<FlowAST> ast);
    
    /**
     * @brief Execute the flow with given parameters
     */
    ExecutionResult execute(const ParameterMap& params = {});
    
    /**
     * @brief Get flow metadata
     */
    const std::string& getTitle() const { return ast_->title; }
    const std::vector<Parameter>& getParameters() const { return ast_->parameters; }
    const std::vector<ReturnValue>& getReturnValues() const { return ast_->returnValues; }
    
    /**
     * @brief Validate flow structure
     */
    std::vector<std::string> validate() const;
    
private:
    std::unique_ptr<FlowAST> ast_;
    
    ExecutionResult executeInternal(ExecutionContext& context);
    FlowNode* findStartNode(ExecutionContext& context);
    std::string getNextNode(const std::string& currentNode, ExecutionContext& context);
    
    // Node execution methods
    void executeAssignNode(const AssignNode& node, ExecutionContext& context);
    std::string executeCondNode(const CondNode& node, ExecutionContext& context);
    void executeProcNode(const ProcNode& node, ExecutionContext& context);
};

/**
 * @brief Main execution engine
 */
class Engine {
public:
    Engine();
    ~Engine();
    
    /**
     * @brief Register external procedure
     */
    void registerProcedure(const std::string& name, ExternalProcedure proc);
    
    /**
     * @brief Create a flow from AST
     */
    Flow createFlow(std::unique_ptr<FlowAST> ast);
    
    /**
     * @brief Get registered procedure
     */
    ExternalProcedure getProcedure(const std::string& name) const;
    
    /**
     * @brief Check if procedure is registered
     */
    bool hasProcedure(const std::string& name) const;
    
private:
    std::unordered_map<std::string, ExternalProcedure> procedures_;
    
    // Built-in procedures
    void registerBuiltinProcedures();
    
    // Built-in procedure implementations
    static ParameterMap builtinPrint(const ParameterMap& params);
    static ParameterMap builtinLog(const ParameterMap& params);
};

} // namespace FlowGraph