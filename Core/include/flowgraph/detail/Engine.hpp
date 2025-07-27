#pragma once

#include "AST.hpp"
#include "Types.hpp"
#include <unordered_map>
#include <memory>
#include <functional>

// Forward declare ExpressionKit types
namespace ExpressionKit {
    class Expression;
    class Context;
}

namespace FlowGraph {

/**
 * @brief Execution state for debugging
 */
enum class ExecutionState {
    NotStarted,    // Execution not started
    Running,       // Currently executing
    Paused,        // Paused for debugging
    Completed,     // Completed successfully
    Error          // Stopped due to error
};

/**
 * @brief Debug step result
 */
struct DebugStepResult {
    ExecutionState state = ExecutionState::Running;
    std::string currentNodeId;
    std::string error;
    ParameterMap localVariables;
    bool flowCompleted = false;
    
    DebugStepResult() = default;
    DebugStepResult(ExecutionState s, const std::string& nodeId) 
        : state(s), currentNodeId(nodeId) {}
    DebugStepResult(const std::string& err) 
        : state(ExecutionState::Error), error(err) {}
};

/**
 * @brief Debug callback function type
 */
using DebugCallback = std::function<void(const DebugStepResult&)>;

/**
 * @brief Execution context for a single flow execution with debugging support
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
    
    // Debugging support
    void setCurrentNode(const std::string& nodeId);
    const std::string& getCurrentNode() const { return currentNodeId_; }
    ParameterMap getLocalVariables() const { return variables_; }
    ExecutionState getState() const { return state_; }
    void setState(ExecutionState state) { state_ = state; }
    
    // Debug callback
    void setDebugCallback(DebugCallback callback) { debugCallback_ = callback; }
    void notifyDebugger() const;
    
private:
    const FlowAST& ast_;
    ParameterMap variables_;
    std::unique_ptr<ExpressionKit::Context> exprContext_;
    
    // Debug state
    ExecutionState state_ = ExecutionState::NotStarted;
    std::string currentNodeId_;
    DebugCallback debugCallback_;
    
    void initializeExpressionContext();
    void updateExpressionContext();
};

/**
 * @brief Debug-enabled execution context
 */
class DebugExecutionContext {
public:
    DebugExecutionContext(std::unique_ptr<ExecutionContext> context);
    
    /**
     * @brief Step to next node
     */
    DebugStepResult step();
    
    /**
     * @brief Continue execution until completion or breakpoint
     */
    ExecutionResult run();
    
    /**
     * @brief Pause execution
     */
    void pause();
    
    /**
     * @brief Get current execution state
     */
    DebugStepResult getCurrentState() const;
    
    /**
     * @brief Get local variables
     */
    ParameterMap getLocalVariables() const;
    
    /**
     * @brief Set variable value during debugging
     */
    void setVariable(const std::string& name, const Value& value);
    
    /**
     * @brief Check if execution is paused
     */
    bool isPaused() const;
    
    /**
     * @brief Check if execution is completed
     */
    bool isCompleted() const;
    
private:
    std::unique_ptr<ExecutionContext> context_;
    bool stepMode_ = false;
    
    DebugStepResult createStepResult() const;
};

/**
 * @brief Loaded and ready-to-execute flow with debugging support
 */
class Flow {
public:
    Flow(std::unique_ptr<FlowAST> ast);
    
    /**
     * @brief Execute the flow with given parameters
     */
    ExecutionResult execute(const ParameterMap& params = {});
    
    /**
     * @brief Create a debug execution context for step-by-step execution
     * @param params Input parameters
     * @return Debug context for controlled execution
     */
    std::unique_ptr<DebugExecutionContext> createDebugContext(const ParameterMap& params = {});
    
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