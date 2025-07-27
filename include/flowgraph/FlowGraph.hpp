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
 * @brief Main FlowGraph engine for loading and executing flows
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
     * @brief Register external procedure
     * @param name Procedure name
     * @param proc Procedure implementation
     */
    void registerProcedure(const std::string& name, ExternalProcedure proc);
    
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

} // namespace FlowGraph