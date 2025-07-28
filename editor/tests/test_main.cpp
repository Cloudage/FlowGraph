// FlowGraph Editor UI Tests
// Main test application for automated UI testing using ImGui Test Engine

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// Include ImGui Test Engine
#include <imgui_test_engine/imgui_te_engine.h>
#include <imgui_test_engine/imgui_te_ui.h>
#include <imgui_test_engine/imgui_te_utils.h>
#include <imgui_test_engine/imgui_te_exporters.h>

// Platform-specific includes
#ifdef __APPLE__
    #include <Metal/Metal.h>
    #include <MetalKit/MetalKit.h>
#elif defined(_WIN32)
    #include <d3d11.h>
    #include <tchar.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

// Platform-specific ImGui backend includes
#ifdef __APPLE__
    #include <imgui_impl_metal.h>
#elif defined(_WIN32)
    #include <imgui_impl_dx11.h>
#else
    #include <imgui_impl_opengl3.h>
#endif

#include <iostream>
#include <memory>

// Include editor app for testing
#include "EditorApp.hpp"

// Forward declarations
void RegisterEditorUITests(ImGuiTestEngine* engine);

// Test application structure
struct UITestApp {
    GLFWwindow* window = nullptr;
    ImGuiTestEngine* test_engine = nullptr;
    FlowGraph::Editor::EditorApp* editor_app = nullptr;
    bool headless_mode = false;
    
    bool Initialize(int argc, char** argv);
    int Run();
    void Shutdown();
    
private:
    bool InitializeWindow();
    bool InitializeImGui();
    bool InitializeTestEngine(int argc, char** argv);
    void ParseCommandLine(int argc, char** argv);
    
    // Rendering backends
#ifdef __APPLE__
    void* metal_device = nullptr;
    void* metal_layer = nullptr;
    void* metal_command_queue = nullptr;
#elif defined(_WIN32)
    ID3D11Device* d3d_device = nullptr;
    ID3D11DeviceContext* d3d_device_context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* main_render_target_view = nullptr;
#endif
};

bool UITestApp::Initialize(int argc, char** argv) {
    ParseCommandLine(argc, argv);
    
    if (!InitializeWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }
    
    if (!InitializeImGui()) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
        return false;
    }
    
    if (!InitializeTestEngine(argc, argv)) {
        std::cerr << "Failed to initialize test engine" << std::endl;
        return false;
    }
    
    return true;
}

bool UITestApp::InitializeWindow() {
    // Set GLFW error callback
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    });
    
    if (!glfwInit()) {
        return false;
    }
    
    // Configure GLFW for different platforms
#ifdef __APPLE__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#elif defined(_WIN32)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    
    // For headless CI testing
    if (headless_mode) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }
    
    // Create window
    window = glfwCreateWindow(1280, 720, "FlowGraph Editor UI Tests", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }
    
#ifndef __APPLE__
#ifndef _WIN32
    glfwMakeContextCurrent(window);
    
    // Initialize OpenGL loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader" << std::endl;
        return false;
    }
#endif
#endif
    
    return true;
}

bool UITestApp::InitializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    
    // Setup ImGui style
    ImGui::StyleColorsDark();
    
    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOther(window, true);
    
#ifdef __APPLE__
    // Metal backend initialization would go here
    // For now, use a minimal setup
    return false; // Metal setup not implemented yet
#elif defined(_WIN32)
    // DirectX 11 backend initialization would go here
    // For now, use a minimal setup
    return false; // DirectX setup not implemented yet
#else
    // OpenGL backend
    ImGui_ImplOpenGL3_Init("#version 330");
#endif
    
    return true;
}

bool UITestApp::InitializeTestEngine(int argc, char** argv) {
    // Create test engine
    test_engine = ImGuiTestEngine_CreateContext();
    if (!test_engine) {
        return false;
    }
    
    // Configure test engine
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(test_engine);
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Fast;
    test_io.ConfigLogToTTY = true;
    test_io.ConfigNoThrottle = true;
    
    // For CI/headless mode
    if (headless_mode) {
        test_io.ConfigCaptureEnabled = false;
        test_io.ConfigMouseDrawCursor = false;
        test_io.ConfigWatchdogKillApp = 30.0f; // Kill after 30 seconds
    }
    
    // Start test engine
    ImGuiTestEngine_Start(test_engine, ImGui::GetCurrentContext());
    
    // Register our UI tests
    RegisterEditorUITests(test_engine);
    
    // Queue tests based on command line
    ImGuiTestEngine_QueueTests(test_engine, ImGuiTestGroup_Tests, nullptr, ImGuiTestRunFlags_RunFromCommandLine);
    
    return true;
}

void UITestApp::ParseCommandLine(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-nogui") == 0 || strcmp(argv[i], "--headless") == 0) {
            headless_mode = true;
        }
    }
}

int UITestApp::Run() {
    bool aborted = false;
    
    while (!glfwWindowShouldClose(window) && !aborted) {
        glfwPollEvents();
        
        // Check if test engine wants to abort
        if (ImGuiTestEngine_TryAbortEngine(test_engine)) {
            break;
        }
        
        // Exit when all tests are done
        if (ImGuiTestEngine_IsTestQueueEmpty(test_engine)) {
            break;
        }
        
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Show test engine windows in GUI mode
        if (!headless_mode) {
            ImGuiTestEngine_ShowTestEngineWindows(test_engine, nullptr);
        }
        
        // Rendering
        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        
        // Required for test engine screen capture
        ImGuiTestEngine_PostSwap(test_engine);
    }
    
    // Get test results
    ImGuiTestEngineResultSummary summary;
    ImGuiTestEngine_GetResultSummary(test_engine, &summary);
    
    std::cout << "UI Tests completed: " << summary.CountSuccess << "/" << summary.CountTested << " passed" << std::endl;
    
    // Return failure if any tests failed
    return (summary.CountSuccess == summary.CountTested) ? 0 : 1;
}

void UITestApp::Shutdown() {
    if (test_engine) {
        ImGuiTestEngine_Stop(test_engine);
        ImGuiTestEngine_DestroyContext(test_engine);
    }
    
    // Cleanup ImGui
    if (ImGui::GetCurrentContext()) {
#ifdef __APPLE__
        // Metal cleanup would go here
#elif defined(_WIN32)
        // DirectX cleanup would go here
#else
        ImGui_ImplOpenGL3_Shutdown();
#endif
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    
    // Cleanup GLFW
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

int main(int argc, char** argv) {
    UITestApp app;
    
    if (!app.Initialize(argc, argv)) {
        std::cerr << "Failed to initialize test application" << std::endl;
        app.Shutdown();
        return 1;
    }
    
    int result = app.Run();
    app.Shutdown();
    
    return result;
}