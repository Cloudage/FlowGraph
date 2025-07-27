// swift-tools-version: 5.7
import PackageDescription

let package = Package(
    name: "FlowGraph",
    platforms: [
        .macOS(.v10_15),
        .iOS(.v13),
        .watchOS(.v6),
        .tvOS(.v13)
    ],
    products: [
        .library(
            name: "FlowGraph",
            targets: ["FlowGraph"]),
    ],
    dependencies: [
        // Add dependencies here if needed
    ],
    targets: [
        .target(
            name: "CFlowGraph",
            dependencies: [],
            path: "Sources/CFlowGraph",
            publicHeadersPath: "include",
            cxxSettings: [
                .define("FLOWGRAPH_SWIFT_PACKAGE"),
                .headerSearchPath("../../../include"),
                .unsafeFlags(["-std=c++17"])
            ]
        ),
        .target(
            name: "FlowGraph",
            dependencies: ["CFlowGraph"],
            path: "Sources/FlowGraph"
        ),
        .testTarget(
            name: "FlowGraphTests",
            dependencies: ["FlowGraph"],
            path: "Tests/FlowGraphTests"
        ),
    ],
    cxxLanguageStandard: .cxx17
)