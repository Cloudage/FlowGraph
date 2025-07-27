#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// C API for FlowGraph to enable Swift interop

typedef struct FlowGraphEngine FlowGraphEngine;
typedef struct FlowGraphFlow FlowGraphFlow;

// Engine management
FlowGraphEngine* flowgraph_engine_create(void);
void flowgraph_engine_destroy(FlowGraphEngine* engine);

// Flow loading and execution
FlowGraphFlow* flowgraph_load_flow(FlowGraphEngine* engine, const char* filepath);
FlowGraphFlow* flowgraph_parse_flow(FlowGraphEngine* engine, const char* content, const char* name);
void flowgraph_flow_destroy(FlowGraphFlow* flow);

// Execution
typedef struct {
    bool success;
    const char* error;
    // TODO: Add parameter/return value handling
} FlowGraphResult;

FlowGraphResult flowgraph_execute_flow(FlowGraphFlow* flow);

// Cleanup
void flowgraph_result_destroy(FlowGraphResult* result);

#ifdef __cplusplus
}
#endif