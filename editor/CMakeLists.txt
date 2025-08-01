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
    set(PLATFORM_LIBS "-framework Metal -framework MetalKit -framework QuartzCore -framework Cocoa -framework IOKit -framework CoreVideo")
elseif(WIN32)
    # DirectX 11 backend for Windows
    list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp)
    
    # DirectX 11 libraries
    set(PLATFORM_LIBS d3d11 dxgi dxguid d3dcompiler)
else()
    # OpenGL 3.3 backend for Linux
    list(APPEND IMGUI_SOURCES ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    
    # Use existing glad target if available, otherwise create one
    if(NOT TARGET glad)
        add_library(glad ${glad_SOURCE_DIR}/src/glad.c)
        target_include_directories(glad PUBLIC ${glad_SOURCE_DIR}/include)
    endif()
    
    set(PLATFORM_LIBS glad)
    find_package(OpenGL REQUIRED)
    list(APPEND PLATFORM_LIBS OpenGL::GL ${CMAKE_DL_LIBS})
endif()

# Create ImGui library
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# Add ImGui Test Engine library if UI tests are enabled
if(BUILD_EDITOR_TESTS)
    # Create separate ImGui library with test engine support
    add_library(imgui_with_tests ${IMGUI_SOURCES})
    target_include_directories(imgui_with_tests PUBLIC 
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui_with_tests PUBLIC glfw)
    # Use custom imconfig.h for test engine
    target_compile_definitions(imgui_with_tests PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h")
    
    # Create ImGui Test Engine library
    set(IMGUI_TEST_ENGINE_SOURCES
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_context.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_coroutine.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_engine.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_exporters.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_perftool.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_ui.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_te_utils.cpp
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine/imgui_capture_tool.cpp
    )
    
    add_library(imgui_test_engine ${IMGUI_TEST_ENGINE_SOURCES})
    target_include_directories(imgui_test_engine PUBLIC 
        ${imgui_test_engine_SOURCE_DIR}
        ${imgui_test_engine_SOURCE_DIR}/imgui_test_engine
    )
    target_link_libraries(imgui_test_engine PUBLIC imgui_with_tests)
    target_compile_definitions(imgui_test_engine PUBLIC 
        IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h"
    )
    target_compile_features(imgui_test_engine PRIVATE cxx_std_17)
endif()

# Platform-specific configuration
if(APPLE)
    # Use Objective-C Metal headers (standard macOS approach)
    # Remove IMGUI_IMPL_METAL_CPP to use Metal.h instead of Metal.hpp
    set_target_properties(imgui PROPERTIES 
        COMPILE_FLAGS "-fobjc-arc"
    )
elseif(WIN32)
    # DirectX 11 configuration for Windows
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_DX11_CPP)
else()
    # OpenGL configuration for Linux
    target_include_directories(imgui PUBLIC ${glad_SOURCE_DIR}/include)
    target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
endif()

# FlowGraph Editor executable - platform-independent core + platform-specific implementations
set(EDITOR_SOURCES
    src/main.cpp
    src/EditorApp.cpp  # Platform-independent core
)

# Add platform-specific implementation files
if(APPLE)
    # Use .mm file for Metal/Objective-C++ support on macOS
    list(APPEND EDITOR_SOURCES src/EditorAppMacOS.mm)
elseif(WIN32)
    # Use .cpp file for DirectX 11 support on Windows
    list(APPEND EDITOR_SOURCES src/EditorAppWindows.cpp)
else()
    # Use .cpp file for OpenGL support on Linux
    list(APPEND EDITOR_SOURCES src/EditorAppLinux.cpp)
endif()

add_executable(FlowGraph_Editor_App ${EDITOR_SOURCES})

# Group in IDE
set_target_properties(FlowGraph_Editor_App PROPERTIES FOLDER "Editor")

target_include_directories(FlowGraph_Editor_App PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Base libraries for the editor
set(EDITOR_LIBS
    FlowGraph::FlowGraph
    imgui
    glfw
    ${PLATFORM_LIBS}
)

target_link_libraries(FlowGraph_Editor_App PRIVATE ${EDITOR_LIBS})

# Set C++ standard
target_compile_features(FlowGraph_Editor_App PRIVATE cxx_std_17)

# Platform-specific settings
if(APPLE)
    set_target_properties(FlowGraph_Editor_App PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
    )
    # Apply ARC to the .mm file only
    set_source_files_properties(src/EditorAppMacOS.mm PROPERTIES
        COMPILE_FLAGS "-fobjc-arc"
    )
endif()

# Add UI tests if enabled
if(BUILD_EDITOR_TESTS)
    # FlowGraph Editor UI Tests executable - use same architecture as main app
    set(UI_TEST_SOURCES
        tests/test_main.cpp
        tests/test_editor_ui.cpp
        src/EditorApp.cpp  # Platform-independent core
    )
    
    # Add platform-specific implementation files for tests
    if(APPLE)
        list(APPEND UI_TEST_SOURCES src/EditorAppMacOS.mm)
    elseif(WIN32)
        list(APPEND UI_TEST_SOURCES src/EditorAppWindows.cpp)
    else()
        list(APPEND UI_TEST_SOURCES src/EditorAppLinux.cpp)
    endif()
    
    add_executable(FlowGraph_Editor_UI_Tests ${UI_TEST_SOURCES})
    
    target_include_directories(FlowGraph_Editor_UI_Tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    
    target_link_libraries(FlowGraph_Editor_UI_Tests PRIVATE
        FlowGraph::FlowGraph
        imgui_test_engine  # This brings in imgui_with_tests
        glfw
        ${PLATFORM_LIBS}
    )
    
    # Enable test engine for UI tests
    target_compile_definitions(FlowGraph_Editor_UI_Tests PRIVATE
        IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h"
    )
    
    target_compile_features(FlowGraph_Editor_UI_Tests PRIVATE cxx_std_17)
    
    # Platform-specific settings for UI tests
    if(APPLE)
        set_target_properties(FlowGraph_Editor_UI_Tests PROPERTIES
            MACOSX_BUNDLE TRUE
            COMPILE_FLAGS "-fobjc-arc"
        )
        # Apply ARC to the .mm file only
        set_source_files_properties(src/EditorAppMacOS.mm PROPERTIES
            COMPILE_FLAGS "-fobjc-arc"
        )
    endif()
    
    # Register UI tests with CTest
    add_test(NAME editor_ui_tests COMMAND xvfb-run -a ${CMAKE_CURRENT_BINARY_DIR}/FlowGraph_Editor_UI_Tests -nogui -nopause)
    set_tests_properties(editor_ui_tests PROPERTIES
        TIMEOUT 60
        LABELS "ui;editor"
    )
    
    # Simple UI test without ImGui Test Engine (more reliable for CI)
    add_executable(FlowGraph_Simple_UI_Test
        tests/simple_ui_test.cpp
    )
    
    target_link_libraries(FlowGraph_Simple_UI_Test PRIVATE
        imgui
        glfw
        ${PLATFORM_LIBS}
    )
    
    target_compile_features(FlowGraph_Simple_UI_Test PRIVATE cxx_std_17)
    
    # Register simple UI test with CTest (primary test for CI)
    add_test(NAME simple_ui_test COMMAND xvfb-run -a ${CMAKE_CURRENT_BINARY_DIR}/FlowGraph_Simple_UI_Test)
    set_tests_properties(simple_ui_test PROPERTIES
        TIMEOUT 30
        LABELS "ui;editor;simple"
    )
endif()