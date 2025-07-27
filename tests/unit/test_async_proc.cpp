#include <catch2/catch_test_macros.hpp>
#include "flowgraph/FlowGraph.hpp"
#include <chrono>
#include <thread>

using namespace FlowGraph;

TEST_CASE("Async PROC functionality", "[async][proc]") {
    FlowGraphEngine engine;
    
    SECTION("Synchronous PROC completes immediately") {
        // Register a synchronous PROC
        auto syncProc = [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            auto input = params.find("input");
            if (input == params.end()) {
                return ProcResult::completedError("Missing input parameter");
            }
            
            ParameterMap result;
            result["output"] = createValue(input->second.asString() + "_processed");
            return ProcResult::completedSuccess(std::move(result));
        };
        
        engine.registerProcedure("sync_proc", syncProc);
        
        // Test execution
        ParameterMap inputs;
        inputs["input"] = createValue("test");
        
        bool callbackCalled = false;
        auto procedure = engine.getProcedure("sync_proc");
        auto result = procedure(inputs, [&callbackCalled](const ProcResult& result) {
            callbackCalled = true;
        });
        
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.returnValues.at("output").asString() == "test_processed");
        REQUIRE_FALSE(callbackCalled); // Callback should not be called for sync completion
    }
    
    SECTION("Asynchronous PROC with callback") {
        // Register an asynchronous PROC
        ProcCompletionCallback storedCallback;
        bool asyncStarted = false;
        
        auto asyncProc = [&](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            auto input = params.find("delay");
            if (input == params.end()) {
                return ProcResult::completedError("Missing delay parameter");
            }
            
            storedCallback = callback;
            asyncStarted = true;
            return ProcResult::pending();
        };
        
        engine.registerProcedure("async_proc", asyncProc);
        
        // Test execution
        ParameterMap inputs;
        inputs["delay"] = createValue(100.0);
        
        bool callbackCalled = false;
        ProcResult callbackResult;
        
        auto procedure = engine.getProcedure("async_proc");
        auto result = procedure(inputs, [&](const ProcResult& result) {
            callbackCalled = true;
            callbackResult = result;
        });
        
        REQUIRE_FALSE(result.completed);
        REQUIRE(asyncStarted);
        REQUIRE_FALSE(callbackCalled);
        
        // Simulate async completion
        ParameterMap asyncResult;
        asyncResult["result"] = createValue("async_completed");
        storedCallback(ProcResult::completedSuccess(std::move(asyncResult)));
        
        REQUIRE(callbackCalled);
        REQUIRE(callbackResult.completed);
        REQUIRE(callbackResult.success);
        REQUIRE(callbackResult.returnValues.at("result").asString() == "async_completed");
    }
    
    SECTION("Legacy synchronous PROC compatibility") {
        // Register a legacy synchronous PROC
        auto legacyProc = [](const ParameterMap& params) -> ParameterMap {
            auto a = params.find("a");
            auto b = params.find("b");
            
            if (a == params.end() || b == params.end()) {
                throw std::runtime_error("Missing parameters");
            }
            
            ParameterMap result;
            result["sum"] = createValue(a->second.asNumber() + b->second.asNumber());
            return result;
        };
        
        engine.registerLegacyProcedure("legacy_add", legacyProc);
        
        // Test execution
        ParameterMap inputs;
        inputs["a"] = createValue(5.0);
        inputs["b"] = createValue(3.0);
        
        auto procedure = engine.getProcedure("legacy_add");
        auto result = procedure(inputs, [](const ProcResult& result) {
            // Should not be called for legacy sync proc
        });
        
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.returnValues.at("sum").asNumber() == 8.0);
    }
    
    SECTION("PROC definition with metadata") {
        ProcDefinition def;
        def.title = "Test Procedure";
        def.parameters = {
            Parameter("input", TypeInfo(ValueType::String), "Input string")
        };
        def.returnValues = {
            ReturnValue("output", TypeInfo(ValueType::String), "Processed output")
        };
        def.errors = {"INVALID_INPUT"};
        def.implementation = [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            auto input = params.find("input");
            if (input == params.end()) {
                return ProcResult::completedError("INVALID_INPUT");
            }
            
            ParameterMap result;
            result["output"] = createValue("processed_" + input->second.asString());
            return ProcResult::completedSuccess(std::move(result));
        };
        
        engine.registerProcedure("test_proc", def);
        
        REQUIRE(engine.hasProcedure("test_proc"));
        
        // Test execution
        ParameterMap inputs;
        inputs["input"] = createValue("hello");
        
        auto procedure = engine.getProcedure("test_proc");
        auto result = procedure(inputs, [](const ProcResult& result) {});
        
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.returnValues.at("output").asString() == "processed_hello");
    }
    
    SECTION("Error handling in async PROC") {
        auto errorProc = [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            auto shouldError = params.find("should_error");
            if (shouldError != params.end() && shouldError->second.asBoolean()) {
                return ProcResult::completedError("Test error condition");
            }
            
            ParameterMap result;
            result["status"] = createValue("success");
            return ProcResult::completedSuccess(std::move(result));
        };
        
        engine.registerProcedure("error_test", errorProc);
        
        // Test error case
        ParameterMap errorInputs;
        errorInputs["should_error"] = createValue(true);
        
        auto procedure = engine.getProcedure("error_test");
        auto errorResult = procedure(errorInputs, [](const ProcResult& result) {});
        
        REQUIRE(errorResult.completed);
        REQUIRE_FALSE(errorResult.success);
        REQUIRE(errorResult.error == "Test error condition");
        
        // Test success case
        ParameterMap successInputs;
        successInputs["should_error"] = createValue(false);
        
        auto successResult = procedure(successInputs, [](const ProcResult& result) {});
        
        REQUIRE(successResult.completed);
        REQUIRE(successResult.success);
        REQUIRE(successResult.returnValues.at("status").asString() == "success");
    }
    
    SECTION("Procedure registry management") {
        // Test initial state
        auto initialProcs = engine.getRegisteredProcedures();
        REQUIRE(initialProcs.size() >= 2); // At least print and log built-ins
        
        // Add a procedure
        engine.registerProcedure("test_registry", [](const ParameterMap& params, ProcCompletionCallback callback) -> ProcResult {
            return ProcResult::completedSuccess({});
        });
        
        REQUIRE(engine.hasProcedure("test_registry"));
        
        auto updatedProcs = engine.getRegisteredProcedures();
        REQUIRE(updatedProcs.size() == initialProcs.size() + 1);
        
        // Check that the new procedure is in the list
        bool found = false;
        for (const auto& name : updatedProcs) {
            if (name == "test_registry") {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }
}

TEST_CASE("ExecutionState enum extension", "[execution][state]") {
    SECTION("WaitingAsync state is available") {
        ExecutionState state = ExecutionState::WaitingAsync;
        REQUIRE(state == ExecutionState::WaitingAsync);
        
        // Test that it's different from other states
        REQUIRE(state != ExecutionState::Running);
        REQUIRE(state != ExecutionState::Paused);
        REQUIRE(state != ExecutionState::Completed);
        REQUIRE(state != ExecutionState::Error);
    }
}

TEST_CASE("DebugStepResult async extensions", "[debug][async]") {
    SECTION("Async fields are properly initialized") {
        DebugStepResult result;
        
        REQUIRE_FALSE(result.waitingForAsync);
        REQUIRE(result.asyncProcName.empty());
        
        // Test setting async state
        result.waitingForAsync = true;
        result.asyncProcName = "test_proc";
        
        REQUIRE(result.waitingForAsync);
        REQUIRE(result.asyncProcName == "test_proc");
    }
}

TEST_CASE("ProcResult factory methods", "[proc][result]") {
    SECTION("completedSuccess factory") {
        ParameterMap values;
        values["result"] = createValue("success");
        
        auto result = ProcResult::completedSuccess(std::move(values));
        
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.error.empty());
        REQUIRE(result.returnValues.at("result").asString() == "success");
    }
    
    SECTION("completedError factory") {
        auto result = ProcResult::completedError("Test error");
        
        REQUIRE(result.completed);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.error == "Test error");
        REQUIRE(result.returnValues.empty());
    }
    
    SECTION("pending factory") {
        auto result = ProcResult::pending();
        
        REQUIRE_FALSE(result.completed);
        REQUIRE(result.success); // Default value
        REQUIRE(result.error.empty());
        REQUIRE(result.returnValues.empty());
    }
}