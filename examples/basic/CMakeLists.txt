# Basic FlowGraph examples

add_executable(hello_example hello_example.cpp)
target_link_libraries(hello_example FlowGraph::FlowGraph)
target_compile_features(hello_example PRIVATE cxx_std_17)