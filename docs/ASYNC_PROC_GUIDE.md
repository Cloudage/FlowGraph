# FlowGraph Async PROC Guide

This guide explains how to use the new asynchronous PROC injection feature in FlowGraph, which allows you to integrate business-related procedures that can be either synchronous or asynchronous.

## Overview

FlowGraph now supports injecting custom PROCs (procedures) that can:
- Complete synchronously (immediately return results)
- Execute asynchronously (return later via callbacks)
- Be transparently handled without user specification of sync/async mode
- Include proper error handling and metadata

## Key Features

### 1. Transparent Sync/Async Handling
The system automatically detects whether a PROC completes synchronously or asynchronously without requiring users to specify the execution model.

### 2. Rich PROC Definitions
PROCs can be defined with complete metadata similar to flow file headers:
- Title and description
- Input parameters with types
- Return values with types  
- Possible error types

### 3. Backward Compatibility
Existing synchronous PROCs continue to work without changes through legacy support.

## Usage Examples

### Basic Synchronous PROC

```cpp
#include "flowgraph/FlowGraph.hpp"

// Simple synchronous PROC
ProcResult getUserInfo(const ParameterMap& params, ProcCompletionCallback callback) {
    auto username = params.at("username").asString();
    
    ParameterMap result;
    result["user_id"] = createValue(static_cast<int64_t>(12345));
    result["email"] = createValue(username + "@example.com");
    
    // Return completed result immediately
    return ProcResult::completedSuccess(std::move(result));
}

// Register the PROC
FlowGraphEngine engine;
engine.registerProcedure("get_user_info", getUserInfo);
```

### Asynchronous PROC with Network Call

```cpp
// Async PROC that simulates network operations
ProcResult fetchWeatherData(const ParameterMap& params, ProcCompletionCallback callback) {
    auto location = params.at("location").asString();
    
    // Start async network operation
    std::thread([callback, location]() {
        // Simulate network delay
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        ParameterMap result;
        result["temperature"] = createValue(25.5);
        result["condition"] = createValue("Sunny");
        
        // Complete via callback
        callback(ProcResult::completedSuccess(std::move(result)));
    }).detach();
    
    // Return pending to indicate async operation
    return ProcResult::pending();
}

engine.registerProcedure("fetch_weather", fetchWeatherData);
```

### PROC with Full Definition

```cpp
// Create a complete PROC definition
ProcDefinition userAuthDef;
userAuthDef.title = "User Authentication";
userAuthDef.parameters = {
    Parameter("username", TypeInfo(ValueType::String), "User login name"),
    Parameter("password", TypeInfo(ValueType::String), "User password")
};
userAuthDef.returnValues = {
    ReturnValue("success", TypeInfo(ValueType::Boolean), "Authentication result"),
    ReturnValue("token", TypeInfo(ValueType::String), "Access token if successful")
};
userAuthDef.errors = {"INVALID_CREDENTIALS", "ACCOUNT_LOCKED", "SERVER_ERROR"};

userAuthDef.implementation = [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
    // Implementation here...
    return ProcResult::completedSuccess({});
};

engine.registerProcedure("authenticate_user", userAuthDef);
```

### Legacy Synchronous PROC Support

```cpp
// Old-style synchronous PROC (automatically wrapped)
ParameterMap legacyCalculate(const ParameterMap& params) {
    double a = params.at("a").asNumber();
    double b = params.at("b").asNumber();
    
    ParameterMap result;
    result["sum"] = createValue(a + b);
    return result;
}

// Register using legacy interface
engine.registerLegacyProcedure("calculate", legacyCalculate);
```

## Real-World Use Cases

### 1. User Interface Interactions

```cpp
ProcResult showUserDialog(const ParameterMap& params, ProcCompletionCallback callback) {
    auto message = params.at("message").asString();
    
    // Show dialog in UI thread
    showDialogAsync(message, [callback](bool userConfirmed, const std::string& userInput) {
        ParameterMap result;
        result["confirmed"] = createValue(userConfirmed);
        result["user_input"] = createValue(userInput);
        
        callback(ProcResult::completedSuccess(std::move(result)));
    });
    
    return ProcResult::pending();
}
```

### 2. Hardware Control

```cpp
ProcResult controlRoboticArm(const ParameterMap& params, ProcCompletionCallback callback) {
    double angle = params.at("angle").asNumber();
    
    // Start hardware movement
    robotArmController.moveToAsync(angle, [callback, angle](bool success, double finalAngle) {
        if (!success) {
            callback(ProcResult::completedError("Hardware movement failed"));
            return;
        }
        
        ParameterMap result;
        result["final_angle"] = createValue(finalAngle);
        result["movement_time"] = createValue(2.5); // seconds
        
        callback(ProcResult::completedSuccess(std::move(result)));
    });
    
    return ProcResult::pending();
}
```

### 3. Network API Calls

```cpp
ProcResult callWebService(const ParameterMap& params, ProcCompletionCallback callback) {
    auto endpoint = params.at("endpoint").asString();
    auto data = params.at("data").asString();
    
    // Make async HTTP request
    httpClient.postAsync(endpoint, data, [callback](const HttpResponse& response) {
        if (response.status != 200) {
            callback(ProcResult::completedError("HTTP error: " + std::to_string(response.status)));
            return;
        }
        
        ParameterMap result;
        result["response_data"] = createValue(response.body);
        result["status_code"] = createValue(static_cast<int64_t>(response.status));
        
        callback(ProcResult::completedSuccess(std::move(result)));
    });
    
    return ProcResult::pending();
}
```

## Error Handling

### Synchronous Errors

```cpp
ProcResult validateInput(const ParameterMap& params, ProcCompletionCallback callback) {
    auto input = params.find("input");
    if (input == params.end()) {
        return ProcResult::completedError("Missing required parameter 'input'");
    }
    
    if (input->second.asString().empty()) {
        return ProcResult::completedError("Input cannot be empty");
    }
    
    // Process valid input...
    return ProcResult::completedSuccess({});
}
```

### Asynchronous Errors

```cpp
ProcResult processDataAsync(const ParameterMap& params, ProcCompletionCallback callback) {
    auto data = params.at("data").asString();
    
    std::thread([callback, data]() {
        try {
            // Process data...
            auto result = processData(data);
            
            ParameterMap output;
            output["result"] = createValue(result);
            callback(ProcResult::completedSuccess(std::move(output)));
        } catch (const std::exception& e) {
            callback(ProcResult::completedError(e.what()));
        }
    }).detach();
    
    return ProcResult::pending();
}
```

## Integration with Flow Files

Once registered, async PROCs can be used in flow files just like any other PROC:

```
TITLE: User Authentication Flow

PARAMS:
S username
S password

RETURNS:
B authenticated
S message

NODES:
10 PROC authenticate_user username>>username password>>password success<<authenticated token<<auth_token
20 COND authenticated
30 ASSIGN S message "Login successful"
40 ASSIGN S message "Login failed"

FLOW:
START -> 10
10 -> 20
20.Y -> 30
20.N -> 40
30 -> END
40 -> END
```

The flow execution engine will automatically handle whether `authenticate_user` completes synchronously or asynchronously.

## Best Practices

### 1. Always Handle Errors
Both sync and async PROCs should properly handle and report errors.

### 2. Use Descriptive Metadata
Include comprehensive titles, parameter descriptions, and error definitions.

### 3. Thread Safety
Ensure async PROCs are thread-safe when accessing shared resources.

### 4. Timeout Handling
Consider implementing timeouts for long-running async operations.

### 5. Resource Management
Properly manage resources in async operations (avoid memory leaks).

## Debugging Async PROCs

The FlowGraph debug system supports async PROCs:

```cpp
auto debugContext = engine.parseFlowForDebugging(flowContent, params);

// Step through execution
while (!debugContext->isCompleted()) {
    auto stepResult = debugContext->step();
    
    if (stepResult.waitingForAsync) {
        std::cout << "Waiting for async PROC: " << stepResult.asyncProcName << std::endl;
        // Handle async completion...
    }
}
```

## Performance Considerations

- Synchronous PROCs have minimal overhead
- Asynchronous PROCs require callback management
- Consider using thread pools for CPU-intensive async operations
- Avoid blocking the main thread in UI applications

## Migration Guide

### From Legacy Synchronous PROCs

Old code:
```cpp
engine.registerProcedure("old_proc", [](const ParameterMap& params) -> ParameterMap {
    // Implementation
    return result;
});
```

New code (still supported):
```cpp
engine.registerLegacyProcedure("old_proc", [](const ParameterMap& params) -> ParameterMap {
    // Same implementation
    return result;
});
```

Or upgrade to new interface:
```cpp
engine.registerProcedure("new_proc", [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
    // Implementation
    return ProcResult::completedSuccess(result);
});
```