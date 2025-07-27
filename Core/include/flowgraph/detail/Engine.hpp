#pragma once

#include "AST.hpp"
#include "Types.hpp"
#include <unordered_map>
#include <memory>
#include <functional>
#include <limits>
#include <cmath>
#include <iostream>
#include <typeinfo>
#include "ExpressionKit.hpp"

namespace FlowGraph {

// Forward declarations
class Engine;
class Flow;
class ExecutionContext;
class DebugExecutionContext;

/**
 * @brief ExpressionKit Environment adapter for FlowGraph ExecutionContext
 */
class ExpressionEnvironment : public ExpressionKit::IEnvironment {
public:
    explicit ExpressionEnvironment(const ParameterMap& variables) : variables_(variables) {}
    
    ExpressionKit::Value Get(const std::string& name) override {
        auto it = variables_.find(name);
        if (it == variables_.end()) {
            throw ExpressionKit::ExprException("Variable not found: " + name);
        }
        return it->second; // Direct return since Value is now ExpressionKit::Value
    }
    
    ExpressionKit::Value Call(const std::string& name, const std::vector<ExpressionKit::Value>& args) override {
        // First try standard mathematical functions
        ExpressionKit::Value result;
        if (ExpressionKit::Expression::CallStandardFunctions(name, args, result)) {
            return result;
        }
        
        // For now, we only support built-in functions
        // TODO: Add support for custom FlowGraph procedures in expressions
        throw ExpressionKit::ExprException("Unknown function: " + name);
    }
    
    void updateVariables(const ParameterMap& variables) {
        variables_ = variables;
    }
    
private:
    ParameterMap variables_;
};

/**
 * @brief Execution state for debugging
 */
enum class ExecutionState {
    NotStarted,      // Execution not started
    Running,         // Currently executing
    Paused,          // Paused for debugging
    WaitingAsync,    // Waiting for async PROC completion
    Completed,       // Completed successfully
    Error            // Stopped due to error
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
    bool waitingForAsync = false;  // true if waiting for async PROC
    std::string asyncProcName;     // name of PROC being waited for
    
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
    ExecutionContext(const FlowAST& ast) : ast_(ast) {
        initializeExpressionEnvironment();
    }
    
    // Variable management
    void setVariable(const std::string& name, const Value& value) {
        variables_[name] = value;
        updateExpressionEnvironment();
    }
    
    Value getVariable(const std::string& name) const {
        auto it = variables_.find(name);
        if (it == variables_.end()) {
            throw FlowGraphError(FlowGraphError::Type::Runtime, "Variable not found: " + name);
        }
        return it->second;
    }
    
    bool hasVariable(const std::string& name) const {
        return variables_.find(name) != variables_.end();
    }
    
    // Parameter binding
    void bindParameters(const ParameterMap& params) {
        for (const auto& [name, value] : params) {
            variables_[name] = value;
        }
        updateExpressionEnvironment();
    }
    
    ParameterMap extractReturnValues() const {
        ParameterMap returnValues;
        for (const auto& retVal : ast_.returnValues) {
            if (hasVariable(retVal.name)) {
                returnValues[retVal.name] = getVariable(retVal.name);
            }
        }
        return returnValues;
    }
    
    // Expression evaluation
    Value evaluateExpression(const std::string& expression) {
        try {
            auto result = ExpressionKit::Expression::Eval(expression, expressionEnv_.get());
            return result; // Direct return since Value is now ExpressionKit::Value
        } catch (const ExpressionKit::ExprException& e) {
            throw FlowGraphError(FlowGraphError::Type::Runtime, "Expression evaluation error: " + std::string(e.what()));
        }
    }
    
    // Debugging support
    void setCurrentNode(const std::string& nodeId) {
        currentNodeId_ = nodeId;
    }
    
    const std::string& getCurrentNode() const { return currentNodeId_; }
    ParameterMap getLocalVariables() const { return variables_; }
    ExecutionState getState() const { return state_; }
    void setState(ExecutionState state) { state_ = state; }
    
    // Async PROC support
    void setWaitingForAsync(const std::string& procName) {
        state_ = ExecutionState::WaitingAsync;
        waitingAsyncProc_ = procName;
    }
    
    void clearAsyncWait() {
        if (state_ == ExecutionState::WaitingAsync) {
            state_ = ExecutionState::Running;
        }
        waitingAsyncProc_.clear();
    }
    
    const std::string& getWaitingAsyncProc() const { return waitingAsyncProc_; }
    bool isWaitingForAsync() const { return state_ == ExecutionState::WaitingAsync; }
    
    // Debug callback
    void setDebugCallback(DebugCallback callback) { debugCallback_ = callback; }
    void notifyDebugger() const {
        if (debugCallback_) {
            DebugStepResult result;
            result.state = state_;
            result.currentNodeId = currentNodeId_;
            result.localVariables = variables_;
            result.flowCompleted = (state_ == ExecutionState::Completed);
            result.waitingForAsync = isWaitingForAsync();
            result.asyncProcName = waitingAsyncProc_;
            debugCallback_(result);
        }
    }
    
private:
    const FlowAST& ast_;
    ParameterMap variables_;
    std::unique_ptr<ExpressionEnvironment> expressionEnv_;
    
    // Debug state
    ExecutionState state_ = ExecutionState::NotStarted;
    std::string currentNodeId_;
    DebugCallback debugCallback_;
    
    // Async state
    std::string waitingAsyncProc_;
    
    void initializeExpressionEnvironment() {
        expressionEnv_ = std::make_unique<ExpressionEnvironment>(variables_);
    }
    
    void updateExpressionEnvironment() {
        if (expressionEnv_) {
            expressionEnv_->updateVariables(variables_);
        }
    }
};

/**
 * @brief Debug-enabled execution context
 */
class DebugExecutionContext {
public:
    DebugExecutionContext(std::unique_ptr<ExecutionContext> context) : context_(std::move(context)) {}
    
    /**
     * @brief Step to next node
     */
    DebugStepResult step() {
        // TODO: Implement step execution logic
        return DebugStepResult(ExecutionState::NotStarted, "");
    }
    
    /**
     * @brief Continue execution until completion or breakpoint
     */
    ExecutionResult run() {
        // TODO: Implement run execution logic
        return ExecutionResult("Debug execution not yet implemented");
    }
    
    /**
     * @brief Pause execution
     */
    void pause() {
        stepMode_ = true;
    }
    
    /**
     * @brief Get current execution state
     */
    DebugStepResult getCurrentState() const {
        return createStepResult();
    }
    
    /**
     * @brief Get local variables
     */
    ParameterMap getLocalVariables() const {
        return context_->getLocalVariables();
    }
    
    /**
     * @brief Set variable value during debugging
     */
    void setVariable(const std::string& name, const Value& value) {
        context_->setVariable(name, value);
    }
    
    /**
     * @brief Check if execution is paused
     */
    bool isPaused() const {
        return context_->getState() == ExecutionState::Paused;
    }
    
    /**
     * @brief Check if execution is completed
     */
    bool isCompleted() const {
        return context_->getState() == ExecutionState::Completed;
    }
    
private:
    std::unique_ptr<ExecutionContext> context_;
    bool stepMode_ = false;
    
    DebugStepResult createStepResult() const {
        DebugStepResult result;
        result.state = context_->getState();
        result.currentNodeId = context_->getCurrentNode();
        result.localVariables = context_->getLocalVariables();
        result.flowCompleted = isCompleted();
        return result;
    }
};

/**
 * @brief Loaded and ready-to-execute flow with debugging support
 */
class Flow {
public:
    Flow(std::unique_ptr<FlowAST> ast, Engine* engine = nullptr) 
        : ast_(std::move(ast)), engine_(engine) {}
    
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
    std::vector<std::string> validate() const {
        return ast_->validate();
    }
    
private:
    std::unique_ptr<FlowAST> ast_;
    Engine* engine_;  // Engine reference for PROC execution
    
    // Method declarations - implementations after Engine class
    ExecutionResult executeInternal(ExecutionContext& context);
    FlowNode* findStartNode(ExecutionContext& context);
    std::string getNextNode(const std::string& currentNode, ExecutionContext& context);
    void executeAssignNode(const AssignNode& node, ExecutionContext& context);
    std::string executeCondNode(const CondNode& node, ExecutionContext& context);
    void executeProcNode(const ProcNode& node, ExecutionContext& context);
    void handleProcResult(const ProcResult& result, const ProcNode& node, ExecutionContext& context);
};

/**
 * @brief Main execution engine
 */
class Engine {
public:
    Engine() {
        registerBuiltinProcedures();
    }
    
    ~Engine() = default;
    
    /**
     * @brief Register external procedure with full definition
     */
    void registerProcedure(const std::string& name, const ProcDefinition& procDef) {
        procedures_[name] = procDef;
    }
    
    /**
     * @brief Register external procedure with implementation only (for backward compatibility)
     */
    void registerProcedure(const std::string& name, ExternalProcedure proc) {
        ProcDefinition def;
        def.title = name;
        def.implementation = proc;
        procedures_[name] = def;
    }
    
    /**
     * @brief Register legacy synchronous procedure (for backward compatibility)
     */
    void registerLegacyProcedure(const std::string& name, LegacyExternalProcedure proc) {
        // Wrap legacy procedure in new async interface
        auto asyncWrapper = [proc](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            (void)callback;  // Suppress unused parameter warning for sync execution
            try {
                // Execute synchronously
                ParameterMap result = proc(params);
                return ProcResult::completedSuccess(std::move(result));
            } catch (const std::exception& e) {
                return ProcResult::completedError(e.what());
            }
        };
        
        registerProcedure(name, asyncWrapper);
    }
    
    /**
     * @brief Create a flow from AST
     */
    Flow createFlow(std::unique_ptr<FlowAST> ast) {
        return Flow(std::move(ast), this);
    }
    
    /**
     * @brief Get registered procedure definition
     */
    const ProcDefinition& getProcedureDefinition(const std::string& name) const {
        auto it = procedures_.find(name);
        if (it == procedures_.end()) {
            throw FlowGraphError(FlowGraphError::Type::Runtime, "Procedure not found: " + name);
        }
        return it->second;
    }
    
    /**
     * @brief Get registered procedure implementation
     */
    ExternalProcedure getProcedure(const std::string& name) const {
        const auto& def = getProcedureDefinition(name);
        return def.implementation;
    }
    
    /**
     * @brief Check if procedure is registered
     */
    bool hasProcedure(const std::string& name) const {
        return procedures_.find(name) != procedures_.end();
    }
    
    /**
     * @brief Get all registered procedure names
     */
    std::vector<std::string> getRegisteredProcedures() const {
        std::vector<std::string> names;
        for (const auto& [name, def] : procedures_) {
            names.push_back(name);
        }
        return names;
    }
    
private:
    std::unordered_map<std::string, ProcDefinition> procedures_;
    
    // Built-in procedures
    void registerBuiltinProcedures() {
        registerLegacyProcedure("print", builtinPrint);
        registerLegacyProcedure("log", builtinLog);
    }
    
    // Built-in procedure implementations
    static ParameterMap builtinPrint(const ParameterMap& params) {
        for (const auto& [name, value] : params) {
            std::cout << name << ": " << value.toString() << std::endl;
        }
        return {};
    }
    
    static ParameterMap builtinLog(const ParameterMap& params) {
        // Similar to print but could log to file in real implementation
        return builtinPrint(params);
    }
};

// Flow method implementations (after Engine class definition)

inline ExecutionResult Flow::execute(const ParameterMap& params) {
    try {
        ExecutionContext context(*ast_);
        context.bindParameters(params);
        return executeInternal(context);
    } catch (const FlowGraphError& e) {
        return ExecutionResult(e.message());
    }
}

inline std::unique_ptr<DebugExecutionContext> Flow::createDebugContext(const ParameterMap& params) {
    auto context = std::make_unique<ExecutionContext>(*ast_);
    context->bindParameters(params);
    return std::make_unique<DebugExecutionContext>(std::move(context));
}

inline ExecutionResult Flow::executeInternal(ExecutionContext& context) {
    try {
        context.setState(ExecutionState::Running);
        
        // Simple sequential execution of nodes for now
        // In a real implementation, this would follow the flow connections
        for (const auto& node : ast_->nodes) {
            if (auto assign = dynamic_cast<const AssignNode*>(node.get())) {
                executeAssignNode(*assign, context);
            } else if (auto cond = dynamic_cast<const CondNode*>(node.get())) {
                executeCondNode(*cond, context);
            } else if (auto proc = dynamic_cast<const ProcNode*>(node.get())) {
                executeProcNode(*proc, context);
                
                // Handle async PROC execution
                if (context.isWaitingForAsync()) {
                    // For non-interactive execution, we can't handle async operations
                    // This would need to be handled at a higher level with event loops
                    return ExecutionResult("Async PROC execution not supported in synchronous mode");
                }
            }
        }
        
        context.setState(ExecutionState::Completed);
        return ExecutionResult(context.extractReturnValues());
    } catch (const std::exception& e) {
        context.setState(ExecutionState::Error);
        return ExecutionResult("Execution error: " + std::string(e.what()));
    }
}

inline FlowNode* Flow::findStartNode(ExecutionContext& context) {
    // TODO: Find the node connected from START
    (void)context;  // Suppress unused parameter warning
    return nullptr;
}

inline std::string Flow::getNextNode(const std::string& currentNode, ExecutionContext& context) {
    // TODO: Find next node based on connections
    (void)currentNode;  // Suppress unused parameter warning
    (void)context;      // Suppress unused parameter warning
    return "END";
}

inline void Flow::executeAssignNode(const AssignNode& node, ExecutionContext& context) {
    // Use ExpressionKit to evaluate the expression
    Value result = context.evaluateExpression(node.expression);
    context.setVariable(node.variableName, result);
}

inline std::string Flow::executeCondNode(const CondNode& node, ExecutionContext& context) {
    // Use ExpressionKit to evaluate the condition
    Value result = context.evaluateExpression(node.condition);
    bool condition = result.asBoolean(); // Use ExpressionKit's asBoolean method
    
    // Find connections based on condition result
    auto connections = ast_->getConnectionsFrom(node.id);
    for (const auto& conn : connections) {
        if ((condition && conn.fromPort == "Y") || 
            (!condition && conn.fromPort == "N") ||
            conn.fromPort.empty()) {
            return conn.toNode;
        }
    }
    
    // Default to END if no matching connection found
    return "END";
}

inline void Flow::executeProcNode(const ProcNode& node, ExecutionContext& context) {
    if (!engine_) {
        throw FlowGraphError(FlowGraphError::Type::Runtime, "No engine available for PROC execution");
    }
    
    if (!engine_->hasProcedure(node.procedureName)) {
        throw FlowGraphError(FlowGraphError::Type::Runtime, "Procedure not found: " + node.procedureName);
    }
    
    // Prepare input parameters from bindings
    ParameterMap inputParams;
    for (const auto& binding : node.bindings) {
        if (!binding.isOutput) { // Input binding (>>)
            if (context.hasVariable(binding.localVar)) {
                inputParams[binding.procParam] = context.getVariable(binding.localVar);
            }
        }
    }
    
    // Execute the PROC
    auto procedure = engine_->getProcedure(node.procedureName);
    
    // Set up completion callback for async handling
    bool asyncCompleted = false;
    ProcResult finalResult;
    
    auto completionCallback = [&asyncCompleted, &finalResult](const ProcResult& result) {
        finalResult = result;
        asyncCompleted = true;
    };
    
    // Execute the PROC
    context.setCurrentNode(node.id);
    ProcResult result = procedure(inputParams, completionCallback);
    
    if (result.completed) {
        // Synchronous completion
        handleProcResult(result, node, context);
    } else {
        // Asynchronous execution - mark context as waiting
        context.setWaitingForAsync(node.procedureName);
        // In a real async execution environment, the completion callback would be called later
        // For this implementation, we'll simulate async behavior but actually wait
        
        // TODO: In a real implementation, this would return here and resume later
        // For now, we'll indicate that async execution is needed
    }
}

inline void Flow::handleProcResult(const ProcResult& result, const ProcNode& node, ExecutionContext& context) {
    if (!result.success) {
        throw FlowGraphError(FlowGraphError::Type::Runtime, 
            "PROC execution failed: " + result.error);
    }
    
    // Map output parameters from bindings
    for (const auto& binding : node.bindings) {
        if (binding.isOutput) { // Output binding (<<)
            auto it = result.returnValues.find(binding.procParam);
            if (it != result.returnValues.end()) {
                context.setVariable(binding.localVar, it->second);
            }
        }
    }
    
    // Clear async wait if it was set
    context.clearAsyncWait();
}

} // namespace FlowGraph