// Linux-specific implementation for EditorApp
#if !defined(__APPLE__) && !defined(_WIN32)

#include "EditorApp.hpp"

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <memory>

namespace FlowGraph {
namespace Editor {

class EditorAppLinux : public EditorApp {
public:
    EditorAppLinux() = default;
    virtual ~EditorAppLinux() {
        if (m_initialized) {
            Shutdown();
        }
    }

    bool Initialize() override {
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
        
        // Initialize demo graph
        InitializeDemoGraph();

        m_initialized = true;
        return true;
    }

    int Run() override {
        if (!m_initialized) {
            std::cerr << "EditorApp not initialized" << std::endl;
            return -1;
        }

        std::cout << "FlowGraph Editor started successfully" << std::endl;
        std::cout << "Platform: Linux (OpenGL 3.3)" << std::endl;

        // Request initial render
        RequestRender();

        // Main application loop with on-demand rendering
        while (ShouldContinue()) {
            // Handle node dragging if active
            if (m_isDraggingNode && m_selectedNodeId != 0) {
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    // Update node position based on mouse movement
                    ImVec2 mouse_pos = ImGui::GetMousePos();
                    auto mouse_graph = ScreenToGraph(mouse_pos);
                    auto graph_pos = flowgraph::layout::Point<double>(
                        mouse_graph.x - m_dragOffset.x, 
                        mouse_graph.y - m_dragOffset.y
                    );
                    
                    if (m_demoGraph) {
                        auto* node = m_demoGraph->getNode(m_selectedNodeId);
                        if (node) {
                            node->position = graph_pos;
                            RequestRender();
                        }
                    }
                } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                    m_isDraggingNode = false;
                }
            }
            
            // Wait for events instead of polling continuously
            // This ensures 0% CPU usage when idle
            glfwWaitEvents();
            
            if (m_shouldRender) {
                RenderFrame();
                m_shouldRender = false;
                
                // Check if ImGui wants to continue rendering (animations, etc.)
                ImGuiIO& io = ImGui::GetIO();
                if (io.WantCaptureMouse || io.WantCaptureKeyboard || m_isDraggingNode || m_isCreatingConnection) {
                    m_shouldRender = true;
                }
            }
        }

        return 0;
    }

    void Shutdown() override {
        if (!m_initialized) {
            return;
        }

        CleanupImGui();
        CleanupWindow();
        
        m_initialized = false;
    }

    void RequestRender() override {
        m_shouldRender = true;
        // Wake up the event loop
        glfwPostEmptyEvent();
    }

    void HandleWindowResize(int width, int height) override {
        // Request immediate re-render
        m_shouldRender = true;
        
        // Wake up the event loop for immediate re-render
        glfwPostEmptyEvent();
    }

    void HandleContentScaleChange(float xscale, float yscale) override {
        m_contentScaleX = xscale;
        m_contentScaleY = yscale;
        
        // Update ImGui font scaling for new DPI
        ImGuiIO& io = ImGui::GetIO();
        if (xscale > 1.0f || yscale > 1.0f) {
            float scale = std::max(xscale, yscale);
            io.FontGlobalScale = scale;
        } else {
            io.FontGlobalScale = 1.0f;
        }
        
        // Update display framebuffer scale
        int windowWidth, windowHeight;
        int framebufferWidth, framebufferHeight;
        glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
        glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
        
        io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);
        if (windowWidth > 0 && windowHeight > 0) {
            io.DisplayFramebufferScale = ImVec2(
                (float)framebufferWidth / windowWidth, 
                (float)framebufferHeight / windowHeight
            );
        }
        
        // Request re-render to apply new scaling
        RequestRender();
    }

protected:
    bool InitializeWindow() override {
        // Setup GLFW
        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });
        
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }

        // Use OpenGL 3.3 for Linux
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

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
        glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* window) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->RequestRender();
        });
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double, double) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->RequestRender();
        });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int, int, int) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->RequestRender();
        });
        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int, int, int, int) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->RequestRender();
        });
        glfwSetWindowFocusCallback(m_window, [](GLFWwindow* window, int) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->RequestRender();
        });
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app && width > 0 && height > 0) app->HandleWindowResize(width, height);
        });
        glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow* window, float xscale, float yscale) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->HandleContentScaleChange(xscale, yscale);
        });

        // Make OpenGL context current
        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable vsync

        return true;
    }

    bool SetupRenderingBackend() override {
        // Initialize GLAD for OpenGL (Linux)
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        return true;
    }

    bool InitializeImGui() override {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        // Enable keyboard and gamepad controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        
        // Configure high-DPI support
        float xscale, yscale;
        glfwGetWindowContentScale(m_window, &xscale, &yscale);
        m_contentScaleX = xscale;
        m_contentScaleY = yscale;
        
        // Configure display size for proper DPI handling
        int windowWidth, windowHeight;
        int framebufferWidth, framebufferHeight;
        glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
        glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
        
        io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);
        if (windowWidth > 0 && windowHeight > 0) {
            io.DisplayFramebufferScale = ImVec2(
                (float)framebufferWidth / windowWidth, 
                (float)framebufferHeight / windowHeight
            );
        }

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup platform/renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        
        // OpenGL 3.3 backend
        const char* glsl_version = "#version 330";
        return ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void RenderFrame() override {
        // Poll events
        glfwPollEvents();
        
        // Update display size and framebuffer scale every frame for accurate rendering
        ImGuiIO& io = ImGui::GetIO();
        int windowWidth, windowHeight;
        int framebufferWidth, framebufferHeight;
        glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
        glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
        
        io.DisplaySize = ImVec2((float)windowWidth, (float)windowHeight);
        if (windowWidth > 0 && windowHeight > 0) {
            io.DisplayFramebufferScale = ImVec2(
                (float)framebufferWidth / windowWidth, 
                (float)framebufferHeight / windowHeight
            );
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New Graph", "Ctrl+N");
                ImGui::MenuItem("Open Graph", "Ctrl+O");
                ImGui::MenuItem("Save Graph", "Ctrl+S");
                ImGui::Separator();
                ImGui::MenuItem("Exit", "Alt+F4");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Layout")) {
                for (const auto& layout : m_availableLayouts) {
                    if (ImGui::MenuItem(layout.c_str(), nullptr, layout == m_currentLayoutAlgorithm)) {
                        m_currentLayoutAlgorithm = layout;
                        ApplyLayout();
                        RequestRender();
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Graph Controls", nullptr, &m_showGraphControls);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Render graph controls panel
        if (m_showGraphControls) {
            RenderGraphControls();
        }
        
        // Render the main graph visualization
        RenderGraph();
        
        // Render status bar at the bottom
        RenderStatusBar();

        // Rendering
        ImGui::Render();
        
        // OpenGL rendering
        int display_w, display_h;
        glfwGetFramebufferSize(m_window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.25f, 0.25f, 0.25f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(m_window);
    }

    bool ShouldContinue() override {
        return !glfwWindowShouldClose(m_window);
    }

    void CleanupImGui() override {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void CleanupWindow() override {
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
    }

    float GetStatusBarHeight() const override {
        return 25.0f * std::max(m_contentScaleX, m_contentScaleY);
    }
};

// Static factory method implementation
std::unique_ptr<EditorApp> EditorApp::create() {
    return std::make_unique<EditorAppLinux>();
}

} // namespace Editor
} // namespace FlowGraph

#endif // !defined(__APPLE__) && !defined(_WIN32)