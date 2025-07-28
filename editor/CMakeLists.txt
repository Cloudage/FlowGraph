cmake_minimum_required(VERSION 3.15)

# Create ImGui library target
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

# Platform-specific ImGui backend
if(APPLE)
    # Metal backend for macOS
    list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_metal.mm)
    set(PLATFORM_LIBS "-framework Metal -framework MetalKit -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(WIN32)
    # DirectX 11 backend for Windows
    list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
    set(PLATFORM_LIBS d3d11.lib d3dcompiler.lib dxgi.lib)
elseif(UNIX AND NOT APPLE)
    # OpenGL 3.3 backend for Linux
    list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    
    # Use existing glad target if available, otherwise create one
    if(NOT TARGET glad)
        add_library(glad ${glad_SOURCE_DIR}/src/glad.c)
        target_include_directories(glad PUBLIC ${glad_SOURCE_DIR}/include)
    endif()
    
    set(PLATFORM_LIBS glad ${CMAKE_DL_LIBS})
    find_package(OpenGL REQUIRED)
    list(APPEND PLATFORM_LIBS OpenGL::GL)
endif()

# Create ImGui library
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# Platform-specific configuration
if(APPLE)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_METAL_CPP)
    set_target_properties(imgui PROPERTIES 
        COMPILE_FLAGS "-fobjc-arc"
    )
elseif(WIN32)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_DX11_CPP)
elseif(UNIX AND NOT APPLE)
    target_include_directories(imgui PUBLIC ${glad_SOURCE_DIR}/include)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

# FlowGraph Editor executable
add_executable(FlowGraphEditor
    src/main.cpp
    src/EditorApp.cpp
)

target_include_directories(FlowGraphEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(FlowGraphEditor PRIVATE
    FlowGraph::FlowGraph
    imgui
    glfw
    ${PLATFORM_LIBS}
)

# Set C++ standard
target_compile_features(FlowGraphEditor PRIVATE cxx_std_17)

# Platform-specific settings
if(APPLE)
    set_target_properties(FlowGraphEditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
endif()