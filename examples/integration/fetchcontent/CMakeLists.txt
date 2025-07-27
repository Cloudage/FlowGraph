cmake_minimum_required(VERSION 3.15)
project(FlowGraphFetchContentExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch FlowGraph using FetchContent
include(FetchContent)

FetchContent_Declare(
    FlowGraph
    GIT_REPOSITORY https://github.com/Cloudage/FlowGraph.git
    GIT_TAG        main
)

# Note: In a real scenario, you would use the above.
# For this example, we'll use the local source since we're already in the repo
FetchContent_Declare(
    FlowGraph
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

FetchContent_MakeAvailable(FlowGraph)

# Create the example executable
add_executable(fetchcontent_example main.cpp)

# Link FlowGraph
target_link_libraries(fetchcontent_example PRIVATE FlowGraph::FlowGraph)

# Ensure C++17
target_compile_features(fetchcontent_example PRIVATE cxx_std_17)