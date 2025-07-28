#include "EditorApp.hpp"

// Platform-specific includes - glad must come before GLFW on Linux
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
#include <imgui.h>
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

namespace FlowGraph {
namespace Editor {

namespace {
    // GLFW error callback
    void GLFWErrorCallback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    // GLFW window refresh callback for on-demand rendering
    void WindowRefreshCallback(GLFWwindow* window) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->RequestRender();
        }
    }
    
    // Cursor position callback to trigger rendering on mouse movement
    void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->RequestRender();
        }
    }
    
    // Mouse button callback to trigger rendering
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->RequestRender();
        }
    }
    
    // Key callback to trigger rendering
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->RequestRender();
        }
    }
    
    // Window focus callback
    void WindowFocusCallback(GLFWwindow* window, int focused) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->RequestRender();
        }
    }
}

EditorApp::EditorApp() = default;

EditorApp::~EditorApp() {
    if (m_initialized) {
        Shutdown();
    }
}

bool EditorApp::Initialize() {
    if (m_initialized) {
        return true;
    }

    if (!InitializeWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }

    if (!SetupRenderingBackend()) {
        std::cerr << "Failed to setup rendering backend" << std::endl;
        CleanupWindow();
        return false;
    }

    if (!InitializeImGui()) {
        std::cerr << "Failed to initialize ImGui" << std::endl;
        CleanupWindow();
        return false;
    }

    m_initialized = true;
    return true;
}

int EditorApp::Run() {
    if (!m_initialized) {
        std::cerr << "EditorApp not initialized" << std::endl;
        return -1;
    }

    std::cout << "FlowGraph Editor started successfully" << std::endl;
    std::cout << "Platform: "
#ifdef __APPLE__
              << "macOS (Metal)"
#elif defined(_WIN32)
              << "Windows (DirectX 11)"
#else
              << "Linux (OpenGL 3.3)"
#endif
              << std::endl;

    // Request initial render
    RequestRender();

    // Main application loop with on-demand rendering
    while (ShouldContinue()) {
        // Wait for events instead of polling continuously
        // This ensures 0% CPU usage when idle
        glfwWaitEvents();
        
        if (m_shouldRender) {
            RenderFrame();
            m_shouldRender = false;
            
            // Check if ImGui wants to continue rendering (animations, etc.)
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                m_shouldRender = true;
            }
        }
    }

    return 0;
}

void EditorApp::Shutdown() {
    if (!m_initialized) {
        return;
    }

    CleanupImGui();
    CleanupWindow();
    
    m_initialized = false;
}

bool EditorApp::InitializeWindow() {
    // Setup GLFW
    glfwSetErrorCallback(GLFWErrorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW window hints based on platform
#ifdef __APPLE__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#elif defined(_WIN32)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set window user pointer for callbacks
    glfwSetWindowUserPointer(m_window, this);
    
    // Set all necessary callbacks for on-demand rendering
    glfwSetWindowRefreshCallback(m_window, WindowRefreshCallback);
    glfwSetCursorPosCallback(m_window, CursorPosCallback);
    glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetWindowFocusCallback(m_window, WindowFocusCallback);

#if !defined(__APPLE__) && !defined(_WIN32)
    // Make OpenGL context current (only for OpenGL backend)
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync
#endif

    return true;
}

bool EditorApp::SetupRenderingBackend() {
#if !defined(__APPLE__) && !defined(_WIN32)
    // Initialize GLAD for OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
#endif
    return true;
}

bool EditorApp::InitializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable keyboard and gamepad controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOther(m_window, true);

#ifdef __APPLE__
    // Metal backend
    ImGui_ImplMetal_Init(MTLCreateSystemDefaultDevice());
#elif defined(_WIN32)
    // DirectX 11 backend
    // Note: This is a simplified setup - production code would need proper D3D11 device creation
    ImGui_ImplDX11_Init(nullptr, nullptr);
#else
    // OpenGL 3.3 backend
    const char* glsl_version = "#version 330";
    ImGui_ImplOpenGL3_Init(glsl_version);
#endif

    return true;
}

void EditorApp::RenderFrame() {
    // Poll events
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplGlfw_NewFrame();

#ifdef __APPLE__
    ImGui_ImplMetal_NewFrame(nullptr);
#elif defined(_WIN32)
    ImGui_ImplDX11_NewFrame();
#else
    ImGui_ImplOpenGL3_NewFrame();
#endif

    ImGui::NewFrame();

    // Show ImGui demo window
    static bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // Show a simple window with app info
    ImGui::Begin("FlowGraph Editor");
    ImGui::Text("Welcome to FlowGraph Editor!");
    ImGui::Text("Platform: "
#ifdef __APPLE__
                "macOS (Metal)"
#elif defined(_WIN32)
                "Windows (DirectX 11)"
#else
                "Linux (OpenGL 3.3)"
#endif
    );
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    
    if (ImGui::Button("Show Demo Window")) {
        show_demo_window = true;
    }
    ImGui::End();

    // Rendering
    ImGui::Render();

#ifdef __APPLE__
    // Metal rendering would go here
    // For now, just clear
#elif defined(_WIN32)
    // DirectX 11 rendering
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#else
    // OpenGL rendering
    int display_w, display_h;
    glfwGetFramebufferSize(m_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(m_window);
#endif
}

bool EditorApp::ShouldContinue() {
    return !glfwWindowShouldClose(m_window);
}

void EditorApp::RequestRender() {
    m_shouldRender = true;
    // Wake up the event loop
    glfwPostEmptyEvent();
}

void EditorApp::CleanupImGui() {
#ifdef __APPLE__
    ImGui_ImplMetal_Shutdown();
#elif defined(_WIN32)
    ImGui_ImplDX11_Shutdown();
#else
    ImGui_ImplOpenGL3_Shutdown();
#endif

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EditorApp::CleanupWindow() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

} // namespace Editor
} // namespace FlowGraph