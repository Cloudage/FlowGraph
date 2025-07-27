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
}