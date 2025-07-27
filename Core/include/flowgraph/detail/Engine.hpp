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

/**
 * @brief Convert FlowGraph Value to ExpressionKit Value
 */
inline ExpressionKit::Value toExpressionKitValue(const Value& value) {
    switch (value.type()) {
        case ValueType::Integer:
            return ExpressionKit::Value(static_cast<double>(value.get<int64_t>()));
        case ValueType::Float:
            return ExpressionKit::Value(value.get<double>());
        case ValueType::Boolean:
            return ExpressionKit::Value(value.get<bool>());
        case ValueType::String:
            return ExpressionKit::Value(value.get<std::string>());
    }
    throw FlowGraphError(FlowGraphError::Type::Type, "Unsupported value type conversion");
}

/**
 * @brief Convert ExpressionKit Value to FlowGraph Value
 */
inline Value fromExpressionKitValue(const ExpressionKit::Value& value) {
    if (value.isNumber()) {
        return Value(value.asNumber());
    } else if (value.isBoolean()) {
        return Value(value.asBoolean());
    } else if (value.isString()) {
        return Value(value.asString());
    }
    throw FlowGraphError(FlowGraphError::Type::Type, "Unsupported ExpressionKit value type conversion");
}

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
        return toExpressionKitValue(it->second);
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
            return fromExpressionKitValue(result);
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
    
    // Debug callback
    void setDebugCallback(DebugCallback callback) { debugCallback_ = callback; }
    void notifyDebugger() const {
        if (debugCallback_) {
            DebugStepResult result;
            result.state = state_;
            result.currentNodeId = currentNodeId_;
            result.localVariables = variables_;
            result.flowCompleted = (state_ == ExecutionState::Completed);
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
    Flow(std::unique_ptr<FlowAST> ast) : ast_(std::move(ast)) {}
    
    /**
     * @brief Execute the flow with given parameters
     */
    ExecutionResult execute(const ParameterMap& params = {}) {
        try {
            ExecutionContext context(*ast_);
            context.bindParameters(params);
            return executeInternal(context);
        } catch (const FlowGraphError& e) {
            return ExecutionResult(e.message());
        }
    }
    
    /**
     * @brief Create a debug execution context for step-by-step execution
     * @param params Input parameters
     * @return Debug context for controlled execution
     */
    std::unique_ptr<DebugExecutionContext> createDebugContext(const ParameterMap& params = {}) {
        auto context = std::make_unique<ExecutionContext>(*ast_);
        context->bindParameters(params);
        return std::make_unique<DebugExecutionContext>(std::move(context));
    }
    
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
    
    ExecutionResult executeInternal(ExecutionContext& context) {
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
                }
            }
            
            context.setState(ExecutionState::Completed);
            return ExecutionResult(context.extractReturnValues());
        } catch (const std::exception& e) {
            context.setState(ExecutionState::Error);
            return ExecutionResult("Execution error: " + std::string(e.what()));
        }
    }
    
    FlowNode* findStartNode(ExecutionContext& context) {
        // TODO: Find the node connected from START
        return nullptr;
    }
    
    std::string getNextNode(const std::string& currentNode, ExecutionContext& context) {
        // TODO: Find next node based on connections
        return "END";
    }
    
    // Node execution methods
    void executeAssignNode(const AssignNode& node, ExecutionContext& context) {
        // Use ExpressionKit to evaluate the expression
        Value result = context.evaluateExpression(node.expression);
        context.setVariable(node.variableName, result);
    }
    
    std::string executeCondNode(const CondNode& node, ExecutionContext& context) {
        // Use ExpressionKit to evaluate the condition
        Value result = context.evaluateExpression(node.condition);
        bool condition = result.toBool();
        
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
    
    void executeProcNode(const ProcNode& node, ExecutionContext& context) {
        // TODO: Implement procedure execution with external procedures
        throw FlowGraphError(FlowGraphError::Type::Runtime, "Procedure execution not yet implemented");
    }
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
     * @brief Register external procedure
     */
    void registerProcedure(const std::string& name, ExternalProcedure proc) {
        procedures_[name] = proc;
    }
    
    /**
     * @brief Create a flow from AST
     */
    Flow createFlow(std::unique_ptr<FlowAST> ast) {
        return Flow(std::move(ast));
    }
    
    /**
     * @brief Get registered procedure
     */
    ExternalProcedure getProcedure(const std::string& name) const {
        auto it = procedures_.find(name);
        if (it == procedures_.end()) {
            throw FlowGraphError(FlowGraphError::Type::Runtime, "Procedure not found: " + name);
        }
        return it->second;
    }
    
    /**
     * @brief Check if procedure is registered
     */
    bool hasProcedure(const std::string& name) const {
        return procedures_.find(name) != procedures_.end();
    }
    
private:
    std::unordered_map<std::string, ExternalProcedure> procedures_;
    
    // Built-in procedures
    void registerBuiltinProcedures() {
        registerProcedure("print", builtinPrint);
        registerProcedure("log", builtinLog);
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

} // namespace FlowGraph