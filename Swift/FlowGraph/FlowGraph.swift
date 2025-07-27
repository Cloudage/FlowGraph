import Foundation

/// Value types supported by FlowGraph (unified with C++ implementation)
public enum FlowGraphValueType {
    case number   // Unified number type (replaces integer/float distinction)
    case boolean
    case string
}

/// Parameter or return value definition
public struct FlowGraphParameter {
    public let name: String
    public let type: FlowGraphValueType
    public let comment: String
    public let optional: Bool
    
    public init(name: String, type: FlowGraphValueType, comment: String = "", optional: Bool = false) {
        self.name = name
        self.type = type
        self.comment = comment
        self.optional = optional
    }
}

/// FlowGraph runtime value - can hold number, boolean, or string
public enum FlowGraphValue {
    case number(Double)
    case boolean(Bool)
    case string(String)
    
    public var type: FlowGraphValueType {
        switch self {
        case .number: return .number
        case .boolean: return .boolean
        case .string: return .string
        }
    }
}

/// Swift wrapper for FlowGraph engine
public class FlowGraphEngine {
    
    public init() {
        // TODO: Initialize C++ engine when implementation is complete
    }
    
    /// Load a flow from a file
    public func loadFlow(from filepath: String) throws -> FlowGraphFlow {
        // TODO: Implement when C++ core is complete
        throw FlowGraphError.loadFailed("C++ implementation not yet connected")
    }
    
    /// Parse a flow from string content
    public func parseFlow(content: String, name: String = "") throws -> FlowGraphFlow {
        // TODO: Implement when C++ core is complete
        throw FlowGraphError.parseFailed("C++ implementation not yet connected")
    }
    
    /// Register an external procedure (for future implementation)
    public func registerProcedure(name: String, implementation: @escaping ([String: FlowGraphValue]) -> [String: FlowGraphValue]) {
        // TODO: Implement when C++ core is complete
    }
}

/// Represents a loaded and ready-to-execute flow
public class FlowGraphFlow {
    
    internal init() {
        // TODO: Implement when C++ core is complete
    }
    
    /// Execute the flow with parameters
    public func execute(parameters: [String: FlowGraphValue] = [:]) -> FlowGraphResult {
        // TODO: Implement when C++ core is complete
        return FlowGraphResult(
            success: false,
            error: "C++ implementation not yet connected",
            returnValues: [:]
        )
    }
    
    /// Get flow metadata (for future implementation)
    public var title: String { "" }
    public var parameters: [FlowGraphParameter] { [] }
    public var returnValues: [FlowGraphParameter] { [] }
}

/// Result of flow execution
public struct FlowGraphResult {
    public let success: Bool
    public let error: String?
    public let returnValues: [String: FlowGraphValue]
    
    internal init(success: Bool, error: String?, returnValues: [String: FlowGraphValue] = [:]) {
        self.success = success
        self.error = error
        self.returnValues = returnValues
    }
}

/// FlowGraph errors
public enum FlowGraphError: Error, LocalizedError {
    case loadFailed(String)
    case parseFailed(String)
    case executionFailed(String)
    
    public var errorDescription: String? {
        switch self {
        case .loadFailed(let message):
            return "Load failed: \(message)"
        case .parseFailed(let message):
            return "Parse failed: \(message)"
        case .executionFailed(let message):
            return "Execution failed: \(message)"
        }
    }
}