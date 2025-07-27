cmake_minimum_required(VERSION 3.15)
project(FlowGraphFindPackageExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FlowGraph package
find_package(FlowGraph REQUIRED)

# Create the example executable
add_executable(findpackage_example main.cpp)

# Link FlowGraph
target_link_libraries(findpackage_example PRIVATE FlowGraph::FlowGraph)

# Ensure C++17
target_compile_features(findpackage_example PRIVATE cxx_std_17)