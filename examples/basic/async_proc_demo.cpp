#include "flowgraph/FlowGraph.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

using namespace FlowGraph;

// Example 1: Synchronous PROC (completes immediately)
ProcResult getUserInfo(const ParameterMap& params, ProcCompletionCallback callback) {
    // This is a synchronous PROC - it completes immediately
    auto username_it = params.find("username");
    if (username_it == params.end()) {
        return ProcResult::completedError("Username parameter missing");
    }
    
    std::string username = username_it->second.asString();
    
    ParameterMap result;
    result["user_id"] = createValue(static_cast<int64_t>(12345));
    result["full_name"] = createValue("John Doe");
    result["email"] = createValue(username + "@example.com");
    
    return ProcResult::completedSuccess(std::move(result));
}

// Example 2: Asynchronous PROC (simulates network call or UI interaction)
std::function<void()> pendingAsyncCallback;

ProcResult fetchWeatherData(const ParameterMap& params, ProcCompletionCallback callback) {
    // This PROC simulates an async network call
    auto location_it = params.find("location");
    if (location_it == params.end()) {
        return ProcResult::completedError("Location parameter missing");
    }
    
    std::string location = location_it->second.asString();
    
    // Store the callback for later completion
    pendingAsyncCallback = [callback, location]() {
        // Simulate getting weather data after some time
        ParameterMap result;
        result["temperature"] = createValue(25.5);
        result["condition"] = createValue("Sunny");
        result["location_name"] = createValue(location);
        
        // Complete the async operation
        callback(ProcResult::completedSuccess(std::move(result)));
    };
    
    std::cout << "Weather request initiated for: " << location << " (async)" << std::endl;
    
    // Return pending to indicate async operation
    return ProcResult::pending();
}

// Example 3: Robotic arm control (long-running operation)
ProcResult controlRoboticArm(const ParameterMap& params, ProcCompletionCallback callback) {
    auto angle_it = params.find("angle");
    if (angle_it == params.end()) {
        return ProcResult::completedError("Angle parameter missing");
    }
    
    double angle = angle_it->second.asNumber();
    
    std::cout << "Starting robotic arm movement to " << angle << " degrees..." << std::endl;
    
    // Simulate the arm movement in a separate thread
    std::thread([callback, angle]() {
        // Simulate 2 seconds of movement
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        ParameterMap result;
        result["final_angle"] = createValue(angle);
        result["movement_time"] = createValue(2.0);
        result["success"] = createValue(true);
        
        std::cout << "Robotic arm movement completed!" << std::endl;
        callback(ProcResult::completedSuccess(std::move(result)));
    }).detach();
    
    return ProcResult::pending();
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
        ReturnValue("user_id", TypeInfo(ValueType::Integer), "Unique user ID"),
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
    auto result1 = userInfoProc(params1, [](const ProcResult& result) {
        std::cout << "Async callback called (but shouldn't be for sync PROC)" << std::endl;
    });
    
    if (result1.completed && result1.success) {
        std::cout << "User info retrieved successfully:" << std::endl;
        for (const auto& [key, value] : result1.returnValues) {
            std::cout << "  " << key << ": " << value.toString() << std::endl;
        }
    } else {
        std::cout << "Error: " << result1.error << std::endl;
    }
    
    // Example 2: Test legacy synchronous PROC
    std::cout << "\n=== Example 2: Legacy Synchronous PROC ===" << std::endl;
    
    ParameterMap params2;
    params2["a"] = createValue(10.0);
    params2["b"] = createValue(5.0);
    
    auto calcProc = engine.getProcedure("calculate");
    auto result2 = calcProc(params2, [](const ProcResult& result) {
        std::cout << "Async callback called (but shouldn't be for sync PROC)" << std::endl;
    });
    
    if (result2.completed && result2.success) {
        std::cout << "Calculation completed:" << std::endl;
        for (const auto& [key, value] : result2.returnValues) {
            std::cout << "  " << key << ": " << value.toString() << std::endl;
        }
    }
    
    // Example 3: Test asynchronous PROC
    std::cout << "\n=== Example 3: Asynchronous PROC ===" << std::endl;
    
    ParameterMap params3;
    params3["location"] = createValue("New York");
    
    auto weatherProc = engine.getProcedure("fetch_weather");
    auto result3 = weatherProc(params3, [](const ProcResult& result) {
        std::cout << "Weather data received asynchronously:" << std::endl;
        if (result.success) {
            for (const auto& [key, value] : result.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Error: " << result.error << std::endl;
        }
    });
    
    if (!result3.completed) {
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
    auto result4 = armProc(params4, [](const ProcResult& result) {
        std::cout << "Robotic arm control completed asynchronously:" << std::endl;
        if (result.success) {
            for (const auto& [key, value] : result.returnValues) {
                std::cout << "  " << key << ": " << value.toString() << std::endl;
            }
        } else {
            std::cout << "Error: " << result.error << std::endl;
        }
    });
    
    if (!result4.completed) {
        std::cout << "Robotic arm movement initiated..." << std::endl;
        
        // Wait for the async operation to complete
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    
    std::cout << "\n=== Demo completed ===" << std::endl;
    
    return 0;
}