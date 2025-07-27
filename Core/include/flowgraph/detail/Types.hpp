#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>
#include "ExpressionKit.hpp"

namespace FlowGraph {

/**
 * @brief Basic data types supported by FlowGraph - mapped to ExpressionKit types
 */
enum class ValueType {
    Integer,    // I - mapped to ExpressionKit NUMBER but with integer semantics
    Float,      // F - mapped to ExpressionKit NUMBER  
    Boolean,    // B - mapped to ExpressionKit BOOLEAN
    String      // S - mapped to ExpressionKit STRING
};

/**
 * @brief Runtime value using ExpressionKit::Value directly
 */
using Value = ExpressionKit::Value;

/**
 * @brief Type information for compile-time checking
 */
struct TypeInfo {
    ValueType type;
    bool optional = false;
    
    TypeInfo(ValueType t, bool opt = false) : type(t), optional(opt) {}
    
    bool matches(const Value& value) const;
    std::string toString() const;
};

/**
 * @brief Parameter definition
 */
struct Parameter {
    std::string name;
    TypeInfo type;
    std::string comment;
    
    Parameter(const std::string& n, TypeInfo t, const std::string& c = "")
        : name(n), type(t), comment(c) {}
};

/**
 * @brief Return value definition
 */
using ReturnValue = Parameter;

/**
 * @brief Map of parameter/variable names to values
 */
using ParameterMap = std::unordered_map<std::string, Value>;

/**
 * @brief PROC execution result for async operations
 */
struct ProcResult {
    bool completed = false;        // true if synchronously completed
    bool success = true;          // execution success status
    std::string error;            // error message if failed
    ParameterMap returnValues;    // return values if completed
    
    // Factory methods
    static ProcResult completedSuccess(ParameterMap values = {}) {
        ProcResult result;
        result.completed = true;
        result.success = true;
        result.returnValues = std::move(values);
        return result;
    }
    
    static ProcResult completedError(const std::string& errorMsg) {
        ProcResult result;
        result.completed = true;
        result.success = false;
        result.error = errorMsg;
        return result;
    }
    
    static ProcResult pending() {
        ProcResult result;
        result.completed = false;
        return result;
    }
};

/**
 * @brief Callback object for async PROC completion that can track resolution status
 */
class ProcCompletionCallback {
public:
    ProcCompletionCallback() = default;
    
    /**
     * @brief Call the callback with a result
     * @param result The PROC execution result
     */
    void operator()(const ProcResult& result) {
        result_ = result;
        resolved_ = true;
        if (callback_) {
            callback_(result);
        }
    }
    
    /**
     * @brief Check if the callback has been resolved (called)
     * @return true if the callback was called
     */
    bool IsResolved() const { return resolved_; }
    
    /**
     * @brief Get the result (only valid if IsResolved() returns true)
     * @return The PROC execution result
     */
    const ProcResult& GetResult() const { return result_; }
    
    /**
     * @brief Set an optional async callback for when the result is available
     * @param callback Function to call when result is available
     */
    void SetAsyncCallback(std::function<void(const ProcResult&)> callback) {
        callback_ = callback;
        if (resolved_ && callback_) {
            callback_(result_);
        }
    }

private:
    bool resolved_ = false;
    ProcResult result_;
    std::function<void(const ProcResult&)> callback_;
};

/**
 * @brief Enhanced external procedure function signature supporting async operations
 * 
 * The PROC should call the completion callback either:
 * 1. Immediately for synchronous operations
 * 2. Later via async mechanism for asynchronous operations
 * 
 * The execution engine checks if the callback IsResolved() immediately after the call.
 */
using ExternalProcedure = std::function<void(const ParameterMap&, ProcCompletionCallback&)>;

/**
 * @brief PROC definition structure similar to flow file headers
 */
struct ProcDefinition {
    std::string title;                    // PROC title/description
    std::vector<Parameter> parameters;    // Input parameters
    std::vector<ReturnValue> returnValues; // Return values
    std::vector<std::string> errors;      // Possible error types
    ExternalProcedure implementation;     // The actual implementation
    
    ProcDefinition() = default;
    ProcDefinition(const std::string& t, 
                   std::vector<Parameter> params, 
                   std::vector<ReturnValue> returns,
                   std::vector<std::string> errs,
                   ExternalProcedure impl)
        : title(t), parameters(std::move(params)), returnValues(std::move(returns)), 
          errors(std::move(errs)), implementation(std::move(impl)) {}
};

/**
 * @brief Legacy external procedure function signature (synchronous only)
 * For backward compatibility with existing synchronous PROCs
 */
using LegacyExternalProcedure = std::function<ParameterMap(const ParameterMap&)>;

/**
 * @brief Execution result containing return values and status
 */
struct ExecutionResult {
    bool success = true;
    std::string error;
    ParameterMap returnValues;
    
    ExecutionResult() = default;
    ExecutionResult(const std::string& err) : success(false), error(err) {}
    ExecutionResult(ParameterMap values) : returnValues(std::move(values)) {}
};

/**
 * @brief Location information for error reporting
 */
struct Location {
    std::string filename;
    size_t line = 0;
    size_t column = 0;
    
    Location() = default;
    Location(const std::string& file, size_t l, size_t c) 
        : filename(file), line(l), column(c) {}
    
    std::string toString() const;
};

/**
 * @brief FlowGraph exception base class
 */
class FlowGraphError : public std::exception {
public:
    enum class Type { Parse, Type, Runtime, IO };
    
    FlowGraphError(Type t, const std::string& msg, std::optional<Location> loc = {})
        : type_(t), message_(msg), location_(loc) {}
    
    Type type() const noexcept { return type_; }
    const char* what() const noexcept override { return message_.c_str(); }
    const std::string& message() const noexcept { return message_; }
    const std::optional<Location>& location() const noexcept { return location_; }
    
private:
    Type type_;
    std::string message_;
    std::optional<Location> location_;
};

// Helper functions for type conversions
ValueType parseValueType(const std::string& typeStr);
std::string valueTypeToString(ValueType type);

// Helper functions to work with ExpressionKit::Value in FlowGraph context
ValueType getValueType(const Value& value);
Value createValue(int64_t value);  // For Integer type semantics
Value createValue(double value);   // For Float type semantics
Value createValue(bool value);     // For Boolean type semantics
Value createValue(const std::string& value); // For String type semantics

// Implementation (header-only)

inline ValueType getValueType(const Value& value) {
    if (value.isNumber()) {
        // Note: ExpressionKit only has NUMBER type, but FlowGraph distinguishes Integer vs Float
        // For now, we'll treat all numbers as Float to maintain compatibility
        // In a more sophisticated implementation, we could store type hints
        return ValueType::Float;
    } else if (value.isBoolean()) {
        return ValueType::Boolean;
    } else if (value.isString()) {
        return ValueType::String;
    }
    throw FlowGraphError(FlowGraphError::Type::Type, "Unknown ExpressionKit value type");
}

inline Value createValue(int64_t value) {
    return Value(static_cast<double>(value)); // Convert to double for ExpressionKit
}

inline Value createValue(double value) {
    return Value(value);
}

inline Value createValue(bool value) {
    return Value(value);
}

inline Value createValue(const std::string& value) {
    // Explicitly create from std::string to avoid const char* ambiguity
    return ExpressionKit::Value(value);
}

// Add overload for const char* to be explicit
inline Value createValue(const char* value) {
    return ExpressionKit::Value(std::string(value));
}

inline bool TypeInfo::matches(const Value& value) const {
    return getValueType(value) == type;
}

inline std::string TypeInfo::toString() const {
    std::string result = valueTypeToString(type);
    if (optional) {
        result += "?";
    }
    return result;
}

inline std::string Location::toString() const {
    if (filename.empty()) {
        return "line " + std::to_string(line) + ", column " + std::to_string(column);
    }
    return filename + ":" + std::to_string(line) + ":" + std::to_string(column);
}

inline ValueType parseValueType(const std::string& typeStr) {
    if (typeStr == "I") return ValueType::Integer;
    if (typeStr == "F") return ValueType::Float;
    if (typeStr == "B") return ValueType::Boolean;
    if (typeStr == "S") return ValueType::String;
    throw FlowGraphError(FlowGraphError::Type::Parse, "Invalid type: " + typeStr);
}

inline std::string valueTypeToString(ValueType type) {
    switch (type) {
        case ValueType::Integer: return "I";
        case ValueType::Float: return "F";
        case ValueType::Boolean: return "B";
        case ValueType::String: return "S";
    }
    return "?";
}

} // namespace FlowGraph