# Basic FlowGraph examples

add_executable(hello_example hello_example.cpp)
target_link_libraries(hello_example FlowGraph::FlowGraph)
target_compile_features(hello_example PRIVATE cxx_std_17)
set_target_properties(hello_example PROPERTIES FOLDER "Examples")

add_executable(flow_expression_demo flow_expression_demo.cpp)
target_link_libraries(flow_expression_demo FlowGraph::FlowGraph)
target_compile_features(flow_expression_demo PRIVATE cxx_std_17)
set_target_properties(flow_expression_demo PROPERTIES FOLDER "Examples")

add_executable(async_proc_demo async_proc_demo.cpp)
target_link_libraries(async_proc_demo FlowGraph::FlowGraph)
target_compile_features(async_proc_demo PRIVATE cxx_std_17)
set_target_properties(async_proc_demo PROPERTIES FOLDER "Examples")