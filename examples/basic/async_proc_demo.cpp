#include "flowgraph/FlowGraph.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

using namespace FlowGraph;

// Example 1: Synchronous PROC (completes immediately)
void getUserInfo(const ParameterMap& params, ProcCompletionCallback& callback) {
    // This is a synchronous PROC - it completes immediately
    auto username_it = params.find("username");
    if (username_it == params.end()) {
        callback(ProcResult::completedError("Username parameter missing"));
        return;
    }
    
    std::string username = username_it->second.asString();
    
    ParameterMap result;
    result["user_id"] = createValue(12345.0);
    result["full_name"] = createValue("John Doe");
    result["email"] = createValue(username + "@example.com");
    
    callback(ProcResult::completedSuccess(std::move(result)));
}

// Example 2: Asynchronous PROC (simulates network call or UI interaction)
std::function<void()> pendingAsyncCallback;

void fetchWeatherData(const ParameterMap& params, ProcCompletionCallback& callback) {
    // This PROC simulates an async network call
    auto location_it = params.find("location");
    if (location_it == params.end()) {
        callback(ProcResult::completedError("Location parameter missing"));
        return;
    }
    
    std::string location = location_it->second.asString();
    
    // Store the callback for later completion
    pendingAsyncCallback = [&callback, location]() {
        // Simulate getting weather data after some time
        ParameterMap result;
        result["temperature"] = createValue(25.5);
        result["condition"] = createValue("Sunny");
        result["location_name"] = createValue(location);
        
        // Complete the async operation
        callback(ProcResult::completedSuccess(std::move(result)));
    };
    
    std::cout << "Weather request initiated for: " << location << " (async)" << std::endl;
    
    // Don't call callback immediately - this makes it async
}

// Example 3: Robotic arm control (long-running operation)
void controlRoboticArm(const ParameterMap& params, ProcCompletionCallback& callback) {
    auto angle_it = params.find("angle");
    if (angle_it == params.end()) {
        callback(ProcResult::completedError("Angle parameter missing"));
        return;
    }
    
    double angle = angle_it->second.asNumber();
    
    std::cout << "Starting robotic arm movement to " << angle << " degrees..." << std::endl;
    
    // Simulate the arm movement in a separate thread
    std::thread([&callback, angle]() {
        // Simulate 2 seconds of movement
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        ParameterMap result;
        result["final_angle"] = createValue(angle);
        result["movement_time"] = createValue(2.0);
        result["success"] = createValue(true);
        
        std::cout << "Robotic arm movement completed!" << std::endl;
        callback(ProcResult::completedSuccess(std::move(result)));
    }).detach();
    
    // Don't call callback immediately - the thread will call it later
}

// Example legacy synchronous PROC (for backward compatibility)
ParameterMap legacyCalculate(const ParameterMap& params) {
    auto a_it = params.find("a");
    auto b_it = params.find("b");
    
    if (a_it == params.end() || b_it == params.end()) {
        throw std::runtime_error("Missing parameters a or b");
    }
    
    double a = a_it->second.asNumber();
    double b = b_it->second.asNumber();
    
    ParameterMap result;
    result["sum"] = createValue(a + b);
    result["product"] = createValue(a * b);
    
    return result;
}

int main() {
    std::cout << "=== FlowGraph Async PROC Demo ===" << std::endl;
    
    FlowGraphEngine engine;
    
    // Register different types of PROCs
    
    // 1. Modern async PROC with full definition
    ProcDefinition userInfoDef;
    userInfoDef.title = "User Information Retrieval";
    userInfoDef.parameters = {
        Parameter("username", TypeInfo(ValueType::String), "User login name")
    };
    userInfoDef.returnValues = {
        ReturnValue("user_id", TypeInfo(ValueType::Number), "Unique user ID"),
        ReturnValue("full_name", TypeInfo(ValueType::String), "User's full name"),
        ReturnValue("email", TypeInfo(ValueType::String), "User's email address")
    };
    userInfoDef.errors = {"USER_NOT_FOUND", "INVALID_USERNAME"};
    userInfoDef.implementation = getUserInfo;
    
    engine.registerProcedure("get_user_info", userInfoDef);
    
    // 2. Async PROC (without full definition for simplicity)
    engine.registerProcedure("fetch_weather", fetchWeatherData);
    engine.registerProcedure("control_arm", controlRoboticArm);
    
    // 3. Legacy synchronous PROC
    engine.registerLegacyProcedure("calculate", legacyCalculate);
    
    std::cout << "\nRegistered procedures:" << std::endl;
    for (const auto& name : engine.getRegisteredProcedures()) {
        std::cout << "- " << name << std::endl;
    }
    
    // Example 1: Test synchronous PROC
    std::cout << "\n=== Example 1: Synchronous PROC ===" << std::endl;
    
    ParameterMap params1;
    params1["username"] = createValue("john_doe");
    
    auto userInfoProc = engine.getProcedure("get_user_info");
    
    ProcCompletionCallback procCallback1;
    procCallback1.SetAsyncCallback([](const ProcResult& result) {
        std::cout << "Async callback called (should be called even for sync PROC)" << std::endl;
    });
    
    userInfoProc(params1, procCallback1);
    
    if (procCallback1.IsResolved()) {
        auto result1 = procCallback1.GetResult();
        if (result1.completed && result1.success) {
            std::cout << "User info retrieved successfully:" << std::endl;
            for (const auto& [key, value] : result1.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Error: " << result1.error << std::endl;
        }
    } else {
        std::cout << "Unexpected async behavior for sync PROC" << std::endl;
    }
    
    // Example 2: Test legacy synchronous PROC
    std::cout << "\n=== Example 2: Legacy Synchronous PROC ===" << std::endl;
    
    ParameterMap params2;
    params2["a"] = createValue(10.0);
    params2["b"] = createValue(5.0);
    
    auto calcProc = engine.getProcedure("calculate");
    
    ProcCompletionCallback procCallback2;
    calcProc(params2, procCallback2);
    
    if (procCallback2.IsResolved()) {
        auto result2 = procCallback2.GetResult();
        if (result2.completed && result2.success) {
            std::cout << "Calculation completed:" << std::endl;
            for (const auto& [key, value] : result2.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        }
    }
    
    // Example 3: Test asynchronous PROC
    std::cout << "\n=== Example 3: Asynchronous PROC ===" << std::endl;
    
    ParameterMap params3;
    params3["location"] = createValue("New York");
    
    auto weatherProc = engine.getProcedure("fetch_weather");
    
    ProcCompletionCallback procCallback3;
    procCallback3.SetAsyncCallback([](const ProcResult& result) {
        std::cout << "Weather data received asynchronously:" << std::endl;
        if (result.success) {
            for (const auto& [key, value] : result.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Error: " << result.error << std::endl;
        }
    });
    
    weatherProc(params3, procCallback3);
    
    if (!procCallback3.IsResolved()) {
        std::cout << "Weather request is pending..." << std::endl;
        
        // Simulate completion after some time
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        if (pendingAsyncCallback) {
            pendingAsyncCallback();
            pendingAsyncCallback = nullptr;
        }
    }
    
    // Example 4: Test robotic arm control (truly async)
    std::cout << "\n=== Example 4: Robotic Arm Control (Truly Async) ===" << std::endl;
    
    ParameterMap params4;
    params4["angle"] = createValue(90.0);
    
    auto armProc = engine.getProcedure("control_arm");
    
    ProcCompletionCallback procCallback4;
    procCallback4.SetAsyncCallback([](const ProcResult& result) {
        std::cout << "Robotic arm control completed asynchronously:" << std::endl;
        if (result.success) {
            for (const auto& [key, value] : result.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Error: " << result.error << std::endl;
        }
    });
    
    armProc(params4, procCallback4);
    
    if (!procCallback4.IsResolved()) {
        std::cout << "Robotic arm movement initiated..." << std::endl;
        
        // Wait for the async operation to complete
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    
    // Example 5: Exception handling in PROCs
    std::cout << "\n=== Example 5: Exception Handling ===" << std::endl;
    {
        // This PROC demonstrates that exceptions are properly caught and converted to error results
        auto faultyProc = [](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto mode = params.find("mode");
            if (mode == params.end()) {
                callback(ProcResult::completedError("Mode parameter missing"));
                return;
            }
            
            std::string modeStr = mode->second.asString();
            if (modeStr == "exception") {
                // This PROC incorrectly throws an exception instead of using callback
                throw std::runtime_error("Simulated unexpected exception in PROC");
            } else if (modeStr == "error") {
                // This is the correct way to handle errors
                callback(ProcResult::completedError("Simulated controlled error"));
                return;
            }
            
            // Normal success case
            ParameterMap result;
            result["result"] = createValue("Success with mode: " + modeStr);
            callback(ProcResult::completedSuccess(std::move(result)));
        };
        
        engine.registerProcedure("faulty_proc", faultyProc);
        auto faultyProcRef = engine.getProcedure("faulty_proc");
        
        // Test 1: Normal operation
        std::cout << "Testing normal operation..." << std::endl;
        ParameterMap normalParams;
        normalParams["mode"] = createValue("normal");
        
        ProcCompletionCallback normalCallback;
        normalCallback.SetAsyncCallback([](const ProcResult& result) {
            if (result.success) {
                std::cout << "Normal operation result: " << result.returnValues.at("result").asString() << std::endl;
            } else {
                std::cout << "Unexpected error: " << result.error << std::endl;
            }
        });
        
        // Simulate the fixed executeProcNode logic with exception handling
        try {
            faultyProcRef(normalParams, normalCallback);
        } catch (const std::exception& e) {
            normalCallback(ProcResult::completedError(e.what()));
        }
        
        // Test 2: Controlled error
        std::cout << "Testing controlled error handling..." << std::endl;
        ParameterMap errorParams;
        errorParams["mode"] = createValue("error");
        
        ProcCompletionCallback errorCallback;
        errorCallback.SetAsyncCallback([](const ProcResult& result) {
            if (result.success) {
                std::cout << "Unexpected success: " << result.returnValues.begin()->second.asString() << std::endl;
            } else {
                std::cout << "Controlled error handled: " << result.error << std::endl;
            }
        });
        
        try {
            faultyProcRef(errorParams, errorCallback);
        } catch (const std::exception& e) {
            errorCallback(ProcResult::completedError(e.what()));
        }
        
        // Test 3: Exception handling
        std::cout << "Testing exception handling..." << std::endl;
        ParameterMap exceptionParams;
        exceptionParams["mode"] = createValue("exception");
        
        ProcCompletionCallback exceptionCallback;
        exceptionCallback.SetAsyncCallback([](const ProcResult& result) {
            if (result.success) {
                std::cout << "Unexpected success: " << result.returnValues.begin()->second.asString() << std::endl;
            } else {
                std::cout << "Exception converted to error: " << result.error << std::endl;
            }
        });
        
        try {
            faultyProcRef(exceptionParams, exceptionCallback);
        } catch (const std::exception& e) {
            exceptionCallback(ProcResult::completedError(e.what()));
        }
    }
    
    std::cout << "\n=== Demo completed ===" << std::endl;
    
    return 0;
}