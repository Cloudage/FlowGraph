#pragma once

#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <optional>
#include <functional>

namespace FlowGraph {

/**
 * @brief Basic data types supported by FlowGraph
 */
enum class ValueType {
    Integer,    // I
    Float,      // F
    Boolean,    // B
    String      // S
};

/**
 * @brief Runtime value that can hold any FlowGraph type
 */
class Value {
public:
    using Variant = std::variant<int64_t, double, bool, std::string>;
    
    Value() = default;
    explicit Value(int64_t value) : data_(value) {}
    explicit Value(double value) : data_(value) {}
    explicit Value(bool value) : data_(value) {}
    explicit Value(const std::string& value) : data_(value) {}
    explicit Value(std::string&& value) : data_(std::move(value)) {}
    explicit Value(const char* value) : data_(std::string(value)) {}
    
    ValueType type() const;
    
    template<typename T>
    T get() const {
        return std::get<T>(data_);
    }
    
    template<typename T>
    bool holds() const {
        return std::holds_alternative<T>(data_);
    }
    
    std::string toString() const;
    bool toBool() const;
    
    // Comparison operators
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;
    
private:
    Variant data_;
};

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
 * @brief External procedure function signature
 */
using ExternalProcedure = std::function<ParameterMap(const ParameterMap&)>;

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

// Implementation (header-only)

inline ValueType Value::type() const {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int64_t>) {
            return ValueType::Integer;
        } else if constexpr (std::is_same_v<T, double>) {
            return ValueType::Float;
        } else if constexpr (std::is_same_v<T, bool>) {
            return ValueType::Boolean;
        } else {
            return ValueType::String;
        }
    }, data_);
}

inline std::string Value::toString() const {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? std::string("true") : std::string("false");
        } else {
            return std::to_string(value);
        }
    }, data_);
}

inline bool Value::toBool() const {
    return std::visit([](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, bool>) {
            return value;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return !value.empty();
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return value != 0;
        } else { // double
            return value != 0.0;
        }
    }, data_);
}

inline bool Value::operator==(const Value& other) const {
    return data_ == other.data_;
}

inline bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

inline bool Value::operator<(const Value& other) const {
    if (type() != other.type()) {
        return false; // Different types are not comparable
    }
    return data_ < other.data_;
}

inline bool Value::operator<=(const Value& other) const {
    return *this < other || *this == other;
}

inline bool Value::operator>(const Value& other) const {
    return !(*this <= other);
}

inline bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

inline bool TypeInfo::matches(const Value& value) const {
    return value.type() == type;
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