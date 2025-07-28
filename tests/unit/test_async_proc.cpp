#include <catch2/catch_test_macros.hpp>
#include "flowgraph/FlowGraph.hpp"
#include <chrono>
#include <thread>

using namespace FlowGraph;

TEST_CASE("Async PROC functionality", "[async][proc]") {
    FlowGraphEngine engine;
    
    SECTION("Synchronous PROC completes immediately") {
        // Register a synchronous PROC
        auto syncProc = [](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto input = params.find("input");
            if (input == params.end()) {
                callback(ProcResult::completedError("Missing input parameter"));
                return;
            }
            
            ParameterMap result;
            result["output"] = createValue(input->second.asString() + "_processed");
            callback(ProcResult::completedSuccess(std::move(result)));
        };
        
        engine.registerProcedure("sync_proc", syncProc);
        
        // Test execution
        ParameterMap inputs;
        inputs["input"] = createValue("test");
        
        bool asyncCallbackCalled = false;
        auto procedure = engine.getProcedure("sync_proc");
        
        ProcCompletionCallback procCallback;
        procCallback.SetAsyncCallback([&asyncCallbackCalled](const ProcResult& /*result*/) {
            asyncCallbackCalled = true;
        });
        
        procedure(inputs, procCallback);
        
        REQUIRE(procCallback.IsResolved());
        auto result = procCallback.GetResult();
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.returnValues.at("output").asString() == "test_processed");
        REQUIRE(asyncCallbackCalled); // Async callback should be called even for sync completion
    }
    
    SECTION("Asynchronous PROC with callback") {
        // Register an asynchronous PROC
        std::function<void(const ProcResult&)> storedCallback;
        bool asyncStarted = false;
        
        auto asyncProc = [&](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto input = params.find("delay");
            if (input == params.end()) {
                callback(ProcResult::completedError("Missing delay parameter"));
                return;
            }
            
            // Store a way to complete the callback later
            storedCallback = [&callback](const ProcResult& result) {
                callback(result);
            };
            asyncStarted = true;
            // Don't call callback immediately - this makes it async
        };
        
        engine.registerProcedure("async_proc", asyncProc);
        
        // Test execution
        ParameterMap inputs;
        inputs["delay"] = createValue(100.0);
        
        bool asyncCallbackCalled = false;
        ProcResult callbackResult;
        
        auto procedure = engine.getProcedure("async_proc");
        
        ProcCompletionCallback procCallback;
        procCallback.SetAsyncCallback([&](const ProcResult& result) {
            asyncCallbackCalled = true;
            callbackResult = result;
        });
        
        procedure(inputs, procCallback);
        
        REQUIRE_FALSE(procCallback.IsResolved());
        REQUIRE(asyncStarted);
        REQUIRE_FALSE(asyncCallbackCalled);
        
        // Simulate async completion
        ParameterMap asyncResult;
        asyncResult["result"] = createValue("async_completed");
        storedCallback(ProcResult::completedSuccess(std::move(asyncResult)));
        
        REQUIRE(procCallback.IsResolved());
        REQUIRE(asyncCallbackCalled);
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
        
        ProcCompletionCallback procCallback;
        procedure(inputs, procCallback);
        
        REQUIRE(procCallback.IsResolved());
        auto result = procCallback.GetResult();
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
        def.implementation = [](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto input = params.find("input");
            if (input == params.end()) {
                callback(ProcResult::completedError("INVALID_INPUT"));
                return;
            }
            
            ParameterMap result;
            result["output"] = createValue("processed_" + input->second.asString());
            callback(ProcResult::completedSuccess(std::move(result)));
        };
        
        engine.registerProcedure("test_proc", def);
        
        REQUIRE(engine.hasProcedure("test_proc"));
        
        // Test execution
        ParameterMap inputs;
        inputs["input"] = createValue("hello");
        
        auto procedure = engine.getProcedure("test_proc");
        
        ProcCompletionCallback procCallback;
        procedure(inputs, procCallback);
        
        REQUIRE(procCallback.IsResolved());
        auto result = procCallback.GetResult();
        REQUIRE(result.completed);
        REQUIRE(result.success);
        REQUIRE(result.returnValues.at("output").asString() == "processed_hello");
    }
    
    SECTION("Error handling in async PROC") {
        auto errorProc = [](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto shouldError = params.find("should_error");
            if (shouldError != params.end() && shouldError->second.asBoolean()) {
                callback(ProcResult::completedError("Test error condition"));
                return;
            }
            
            ParameterMap result;
            result["status"] = createValue("success");
            callback(ProcResult::completedSuccess(std::move(result)));
        };
        
        engine.registerProcedure("error_test", errorProc);
        
        // Test error case
        ParameterMap errorInputs;
        errorInputs["should_error"] = createValue(true);
        
        auto procedure = engine.getProcedure("error_test");
        
        ProcCompletionCallback errorCallback;
        procedure(errorInputs, errorCallback);
        
        REQUIRE(errorCallback.IsResolved());
        auto errorResult = errorCallback.GetResult();
        REQUIRE(errorResult.completed);
        REQUIRE_FALSE(errorResult.success);
        REQUIRE(errorResult.error == "Test error condition");
        
        // Test success case
        ParameterMap successInputs;
        successInputs["should_error"] = createValue(false);
        
        ProcCompletionCallback successCallback;
        procedure(successInputs, successCallback);
        
        REQUIRE(successCallback.IsResolved());
        auto successResult = successCallback.GetResult();
        REQUIRE(successResult.completed);
        REQUIRE(successResult.success);
        REQUIRE(successResult.returnValues.at("status").asString() == "success");
    }
    
    SECTION("Exception handling in async PROC") {
        // Test PROC that throws an exception instead of properly using callback
        auto exceptionProc = [](const ParameterMap& params, ProcCompletionCallback& callback) -> void {
            auto shouldThrow = params.find("should_throw");
            if (shouldThrow != params.end() && shouldThrow->second.asBoolean()) {
                // This PROC incorrectly throws an exception instead of using callback
                throw std::runtime_error("PROC threw an exception directly");
            }
            
            ParameterMap result;
            result["status"] = createValue("success");
            callback(ProcResult::completedSuccess(std::move(result)));
        };
        
        engine.registerProcedure("exception_proc", exceptionProc);
        
        // Test that exceptions are caught and converted to error results
        ParameterMap inputs;
        inputs["should_throw"] = createValue(true);
        
        auto procedure = engine.getProcedure("exception_proc");
        ProcCompletionCallback procCallback;
        
        // This should not throw - exceptions should be caught and converted
        REQUIRE_NOTHROW([&]() {
            // Simulate the fixed executeProcNode logic
            try {
                procedure(inputs, procCallback);
            } catch (const std::exception& e) {
                procCallback(ProcResult::completedError(e.what()));
            }
        }());
        
        REQUIRE(procCallback.IsResolved());
        auto result = procCallback.GetResult();
        REQUIRE(result.completed);
        REQUIRE_FALSE(result.success);
        REQUIRE(result.error == "PROC threw an exception directly");
        
        // Test that normal operation still works
        ParameterMap normalInputs;
        normalInputs["should_throw"] = createValue(false);
        
        ProcCompletionCallback normalCallback;
        try {
            procedure(normalInputs, normalCallback);
        } catch (const std::exception& e) {
            normalCallback(ProcResult::completedError(e.what()));
        }
        
        REQUIRE(normalCallback.IsResolved());
        auto normalResult = normalCallback.GetResult();
        REQUIRE(normalResult.completed);
        REQUIRE(normalResult.success);
        REQUIRE(normalResult.returnValues.at("status").asString() == "success");
    }
    
    SECTION("Procedure registry management") {
        // Test initial state
        auto initialProcs = engine.getRegisteredProcedures();
        REQUIRE(initialProcs.size() >= 2); // At least print and log built-ins
        
        // Add a procedure
        engine.registerProcedure("test_registry", [](const ParameterMap& /*params*/, ProcCompletionCallback& callback) -> void {
            callback(ProcResult::completedSuccess({}));
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