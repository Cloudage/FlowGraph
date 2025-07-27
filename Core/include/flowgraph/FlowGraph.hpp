#pragma once

/**
 * @file FlowGraph.hpp
 * @brief Main header for the FlowGraph library
 * 
 * FlowGraph is a header-only C++ library for parsing and executing
 * text-based flowcharts. It provides a simple, embeddable flowchart
 * execution engine designed for games and applications.
 */

#include "flowgraph/detail/Types.hpp"
#include "flowgraph/detail/AST.hpp"
#include "flowgraph/detail/Parser.hpp"
#include "flowgraph/detail/Engine.hpp"

namespace FlowGraph {

/**
 * @brief Main FlowGraph engine for loading and executing flows with debugging support
 */
class FlowGraphEngine {
public:
    /**
     * @brief Load a flow from file
     * @param filepath Path to the .flow file
     * @return Loaded flow ready for execution
     */
    Flow loadFlow(const std::string& filepath);
    
    /**
     * @brief Parse a flow from string content
     * @param content Flow content as string
     * @param name Optional name for the flow
     * @return Loaded flow ready for execution
     */
    Flow parseFlow(const std::string& content, const std::string& name = "");
    
    /**
     * @brief Register external procedure with full definition
     * @param name Procedure name
     * @param procDef Complete procedure definition including metadata
     */
    void registerProcedure(const std::string& name, const ProcDefinition& procDef);
    
    /**
     * @brief Register external procedure with implementation only
     * @param name Procedure name
     * @param proc Procedure implementation
     */
    void registerProcedure(const std::string& name, ExternalProcedure proc);
    
    /**
     * @brief Register legacy synchronous external procedure (for backward compatibility)
     * @param name Procedure name
     * @param proc Legacy synchronous procedure implementation
     */
    void registerLegacyProcedure(const std::string& name, LegacyExternalProcedure proc);
    
    /**
     * @brief Get procedure implementation for testing purposes
     * @param name Procedure name
     * @return Procedure implementation
     */
    ExternalProcedure getProcedure(const std::string& name);
    
    /**
     * @brief Check if procedure is registered
     * @param name Procedure name
     * @return True if procedure is registered
     */
    bool hasProcedure(const std::string& name);
    
    /**
     * @brief Get all registered procedure names
     * @return Vector of procedure names
     */
    std::vector<std::string> getRegisteredProcedures();
    
    /**
     * @brief Load a flow and create debug context
     * @param filepath Path to the .flow file
     * @param params Input parameters
     * @return Debug execution context for step-by-step execution
     */
    std::unique_ptr<DebugExecutionContext> loadFlowForDebugging(
        const std::string& filepath, 
        const ParameterMap& params = {}
    );
    
    /**
     * @brief Parse a flow and create debug context
     * @param content Flow content as string
     * @param params Input parameters
     * @param name Optional name for the flow
     * @return Debug execution context for step-by-step execution
     */
    std::unique_ptr<DebugExecutionContext> parseFlowForDebugging(
        const std::string& content, 
        const ParameterMap& params = {},
        const std::string& name = ""
    );
    
private:
    Parser parser_;
    Engine engine_;
};

/**
 * @brief Convenience function to execute a flow file
 * @param filepath Path to the .flow file
 * @param params Input parameters
 * @return Execution result
 */
ExecutionResult executeFlow(const std::string& filepath, const ParameterMap& params = {});

// Inline implementations for FlowGraphEngine methods

inline void FlowGraphEngine::registerProcedure(const std::string& name, const ProcDefinition& procDef) {
    engine_.registerProcedure(name, procDef);
}

inline void FlowGraphEngine::registerProcedure(const std::string& name, ExternalProcedure proc) {
    engine_.registerProcedure(name, proc);
}

inline void FlowGraphEngine::registerLegacyProcedure(const std::string& name, LegacyExternalProcedure proc) {
    engine_.registerLegacyProcedure(name, proc);
}

inline ExternalProcedure FlowGraphEngine::getProcedure(const std::string& name) {
    return engine_.getProcedure(name);
}

inline bool FlowGraphEngine::hasProcedure(const std::string& name) {
    return engine_.hasProcedure(name);
}

inline std::vector<std::string> FlowGraphEngine::getRegisteredProcedures() {
    return engine_.getRegisteredProcedures();
}

} // namespace FlowGraph