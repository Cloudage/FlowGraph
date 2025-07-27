import XCTest
@testable import FlowGraph

final class FlowGraphTests: XCTestCase {
    
    func testEngineCreation() {
        // Test that we can create an engine without crashing
        let engine = FlowGraphEngine()
        XCTAssertNotNil(engine)
    }
    
    func testBasicFlowCreation() {
        // TODO: This will be implemented when the C++ core is complete
        // For now, just test that the Swift wrapper compiles and runs
        XCTAssertTrue(true)
    }
    
    // More comprehensive tests will be added as the library is developed
}