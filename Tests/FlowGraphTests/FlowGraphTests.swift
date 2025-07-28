import XCTest
@testable import FlowGraph

final class FlowGraphTests: XCTestCase {
    
    func testEngineCreation() {
        // Test that we can create an engine without crashing
        let engine = FlowGraphEngine()
        XCTAssertNotNil(engine)
    }
    
    func testValueTypes() {
        // Test FlowGraphValue type system
        let numberValue = FlowGraphValue.number(42.0)
        let boolValue = FlowGraphValue.boolean(true)
        let stringValue = FlowGraphValue.string("hello")
        
        XCTAssertEqual(numberValue.type, .number)
        XCTAssertEqual(boolValue.type, .boolean)
        XCTAssertEqual(stringValue.type, .string)
    }
    
    func testParameterDefinition() {
        // Test parameter definition structure
        let param = FlowGraphParameter(
            name: "input", 
            type: .number, 
            comment: "Test input parameter",
            optional: false
        )
        
        XCTAssertEqual(param.name, "input")
        XCTAssertEqual(param.type, .number)
        XCTAssertEqual(param.comment, "Test input parameter")
        XCTAssertFalse(param.optional)
    }
    
    func testFlowExecutionStub() {
        // Test that the stub implementation works correctly
        let engine = FlowGraphEngine()
        
        do {
            _ = try engine.loadFlow(from: "test.flow")
            XCTFail("Should have thrown an error")
        } catch FlowGraphError.loadFailed(let message) {
            XCTAssertEqual(message, "C++ implementation not yet connected")
        } catch {
            XCTFail("Wrong error type")
        }
        
        do {
            _ = try engine.parseFlow(content: "TITLE: Test", name: "test")
            XCTFail("Should have thrown an error")
        } catch FlowGraphError.parseFailed(let message) {
            XCTAssertEqual(message, "C++ implementation not yet connected")
        } catch {
            XCTFail("Wrong error type")
        }
    }
    
    // Test flow execution with parameters
    func testFlowExecutionWithParameters() {
        // This will test the API design even though implementation is stubbed
        let flow = FlowGraphFlow()
        let parameters: [String: FlowGraphValue] = [
            "input": .number(42.0),
            "flag": .boolean(true),
            "name": .string("test")
        ]
        
        let result = flow.execute(parameters: parameters)
        XCTAssertFalse(result.success)
        XCTAssertEqual(result.error, "C++ implementation not yet connected")
        XCTAssertTrue(result.returnValues.isEmpty)
    }
    
    func testValueTypeSystem() {
        // Test comprehensive value type coverage
        let values: [FlowGraphValue] = [
            .number(0.0),
            .number(-42.5),
            .number(1e10),
            .boolean(true),
            .boolean(false),
            .string(""),
            .string("Hello, World! 🚀"),
            .string("Multi\nLine\tString")
        ]
        
        for value in values {
            // Ensure type consistency
            switch value {
            case .number(_):
                XCTAssertEqual(value.type, .number)
            case .boolean(_):
                XCTAssertEqual(value.type, .boolean)
            case .string(_):
                XCTAssertEqual(value.type, .string)
            }
        }
    }
    
    func testParameterDefinitionEdgeCases() {
        // Test parameter definitions with edge cases
        let params = [
            FlowGraphParameter(name: "", type: .string, comment: "", optional: true),
            FlowGraphParameter(name: "very_long_parameter_name_with_underscores", type: .number),
            FlowGraphParameter(name: "param", type: .boolean, comment: "Multi\nline\ncomment", optional: false),
        ]
        
        XCTAssertEqual(params[0].name, "")
        XCTAssertTrue(params[0].optional)
        
        XCTAssertEqual(params[1].name, "very_long_parameter_name_with_underscores")
        XCTAssertEqual(params[1].type, .number)
        
        XCTAssertEqual(params[2].comment, "Multi\nline\ncomment")
        XCTAssertFalse(params[2].optional)
    }
    
    func testErrorHandlingComprehensive() {
        // Test comprehensive error handling
        let engine = FlowGraphEngine()
        
        // Test multiple error conditions
        let testCases = [
            ("", "Empty filename"),
            ("nonexistent.flow", "Nonexistent file"),
            ("/invalid/path/file.flow", "Invalid path")
        ]
        
        for (filename, description) in testCases {
            do {
                _ = try engine.loadFlow(from: filename)
                XCTFail("Should have thrown error for: \(description)")
            } catch FlowGraphError.loadFailed(let message) {
                XCTAssertEqual(message, "C++ implementation not yet connected")
            } catch {
                XCTFail("Wrong error type for: \(description)")
            }
        }
        
        // Test parse flow error conditions
        let parseTestCases = [
            ("", ""),
            ("INVALID CONTENT", "invalid"),
            ("TITLE: Test\nINVALID", "test_partial")
        ]
        
        for (content, name) in parseTestCases {
            do {
                _ = try engine.parseFlow(content: content, name: name)
                XCTFail("Should have thrown error for content: \(content)")
            } catch FlowGraphError.parseFailed(let message) {
                XCTAssertEqual(message, "C++ implementation not yet connected")
            } catch {
                XCTFail("Wrong error type for content: \(content)")
            }
        }
    }
}