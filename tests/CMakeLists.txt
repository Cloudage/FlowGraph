cmake_minimum_required(VERSION 3.15)

# Fetch Catch2 - hide from IDE targets
include(FetchContent)
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Hide Catch2 targets from IDE to reduce clutter
set_target_properties(Catch2 PROPERTIES EXCLUDE_FROM_ALL TRUE FOLDER "Dependencies")
set_target_properties(Catch2WithMain PROPERTIES EXCLUDE_FROM_ALL TRUE FOLDER "Dependencies")

# Test executable
add_executable(FlowGraphTests
    test_main.cpp
    unit/test_types.cpp
    unit/test_parser.cpp
    unit/test_engine.cpp
    unit/test_ast.cpp
    unit/test_expression_integration.cpp
    unit/test_async_proc.cpp
    unit/test_layout.cpp
    unit/test_editor_integration.cpp
    integration/test_basic_flows.cpp
    integration/test_examples.cpp
)

# Group in IDE
set_target_properties(FlowGraphTests PROPERTIES FOLDER "Tests")

target_link_libraries(FlowGraphTests
    PRIVATE
    FlowGraph::FlowGraph
    Catch2::Catch2WithMain
)

target_compile_features(FlowGraphTests PRIVATE cxx_std_17)

# Add compiler warnings
if(MSVC)
    target_compile_options(FlowGraphTests PRIVATE /W4)
else()
    target_compile_options(FlowGraphTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Register tests with CTest
include(Catch)
catch_discover_tests(FlowGraphTests)