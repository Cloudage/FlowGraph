import CFlowGraph
import Foundation

/// Swift wrapper for FlowGraph engine
public class FlowGraphEngine {
    private let engine: OpaquePointer
    
    public init() {
        engine = flowgraph_engine_create()
    }
    
    deinit {
        flowgraph_engine_destroy(engine)
    }
    
    /// Load a flow from a file
    public func loadFlow(from filepath: String) throws -> FlowGraphFlow {
        guard let flow = flowgraph_load_flow(engine, filepath) else {
            throw FlowGraphError.loadFailed("Failed to load flow from: \(filepath)")
        }
        return FlowGraphFlow(flow: flow)
    }
    
    /// Parse a flow from string content
    public func parseFlow(content: String, name: String = "") throws -> FlowGraphFlow {
        guard let flow = flowgraph_parse_flow(engine, content, name) else {
            throw FlowGraphError.parseFailed("Failed to parse flow content")
        }
        return FlowGraphFlow(flow: flow)
    }
}

/// Represents a loaded and ready-to-execute flow
public class FlowGraphFlow {
    private let flow: OpaquePointer
    
    internal init(flow: OpaquePointer) {
        self.flow = flow
    }
    
    deinit {
        flowgraph_flow_destroy(flow)
    }
    
    /// Execute the flow
    public func execute() -> FlowGraphResult {
        let result = flowgraph_execute_flow(flow)
        return FlowGraphResult(
            success: result.success,
            error: result.error.map { String(cString: $0) }
        )
    }
}

/// Result of flow execution
public struct FlowGraphResult {
    public let success: Bool
    public let error: String?
    
    internal init(success: Bool, error: String?) {
        self.success = success
        self.error = error
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