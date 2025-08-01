// macOS-specific implementation for EditorApp
#ifdef __APPLE__

#include "EditorApp.hpp"

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <QuartzCore/QuartzCore.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_metal.h>

#include <iostream>
#include <memory>

namespace FlowGraph {
namespace Editor {

class EditorAppMacOS : public EditorApp {
private:
    // Metal-specific members for macOS
    id<MTLDevice> m_metalDevice = nullptr;
    id<MTLCommandQueue> m_metalCommandQueue = nullptr;
    CAMetalLayer* m_metalLayer = nullptr;
    
    // Store the current drawable for the frame
    id<CAMetalDrawable> m_currentDrawable = nil;

public:
    EditorAppMacOS() = default;
    virtual ~EditorAppMacOS() {
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
        std::cout << "Platform: macOS (Metal)" << std::endl;

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
        
        // Update Metal layer drawable size
        if (m_metalLayer) {
            UpdateMetalLayerSize(width, height);
        }
        
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

        // Configure GLFW window hints for Metal
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

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

        return true;
    }

    bool SetupRenderingBackend() override {
        @autoreleasepool {
            // Metal setup for macOS
            // Create Metal device
            m_metalDevice = MTLCreateSystemDefaultDevice();
            if (!m_metalDevice) {
                std::cerr << "Failed to create Metal device" << std::endl;
                return false;
            }
            
            // Create command queue
            m_metalCommandQueue = [m_metalDevice newCommandQueue];
            if (!m_metalCommandQueue) {
                std::cerr << "Failed to create Metal command queue" << std::endl;
                return false;
            }
            
            // Create Metal layer for the GLFW window
            NSWindow* nsWindow = glfwGetCocoaWindow(m_window);
            m_metalLayer = [CAMetalLayer layer];
            m_metalLayer.device = m_metalDevice;
            m_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            // Get window dimensions for proper drawable size
            int windowWidth, windowHeight;
            int framebufferWidth, framebufferHeight;
            glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
            glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
            
            // Set the drawable size to match framebuffer (for Retina displays)
            m_metalLayer.drawableSize = CGSizeMake(framebufferWidth, framebufferHeight);
            
            NSView* contentView = [nsWindow contentView];
            [contentView setWantsLayer:YES];
            [contentView setLayer:m_metalLayer];
            
            std::cout << "Metal initialized successfully" << std::endl;
            return true;
        }
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
        ImGui_ImplGlfw_InitForOther(m_window, true);
        
        // Metal backend
        return ImGui_ImplMetal_Init(m_metalDevice);
    }

    void RenderFrame() override {
        @autoreleasepool {
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

            // Get drawable and start Metal frame
            m_currentDrawable = [m_metalLayer nextDrawable];
            if (!m_currentDrawable) {
                return; // Skip frame if no drawable available
            }
            
            MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDescriptor.colorAttachments[0].texture = m_currentDrawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.25, 0.25, 0.25, 1.0);
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            
            // Start ImGui frame
            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Menu bar
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    ImGui::MenuItem("New Graph", "Cmd+N");
                    ImGui::MenuItem("Open Graph", "Cmd+O");
                    ImGui::MenuItem("Save Graph", "Cmd+S");
                    ImGui::Separator();
                    ImGui::MenuItem("Exit", "Cmd+Q");
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
            
            // Metal rendering
            id<MTLCommandBuffer> commandBuffer = [m_metalCommandQueue commandBuffer];
            if (!commandBuffer) {
                return;
            }
            
            // Create render encoder 
            id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            if (!renderEncoder) {
                return;
            }
            
            // Check if ImGui has valid draw data before rendering
            ImDrawData* drawData = ImGui::GetDrawData();
            if (drawData && drawData->Valid && drawData->CmdListsCount > 0) {
                // Render ImGui
                ImGui_ImplMetal_RenderDrawData(drawData, commandBuffer, renderEncoder);
            }
            
            // End encoding
            [renderEncoder endEncoding];
            
            // Present drawable and commit
            [commandBuffer presentDrawable:m_currentDrawable];
            [commandBuffer commit];
            
            // Clear the drawable for next frame
            m_currentDrawable = nil;
        }
    }

    bool ShouldContinue() override {
        return !glfwWindowShouldClose(m_window);
    }

    void CleanupImGui() override {
        ImGui_ImplMetal_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void CleanupWindow() override {
        // Cleanup Metal resources
        if (m_metalLayer) {
            m_metalLayer = nullptr;
        }
        if (m_metalCommandQueue) {
            m_metalCommandQueue = nullptr;
        }
        if (m_metalDevice) {
            m_metalDevice = nullptr;
        }
        
        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
    }

    float GetStatusBarHeight() const override {
        return 12.0f * std::max(m_contentScaleX, m_contentScaleY);
    }

private:
    void UpdateMetalLayerSize(int width, int height) {
        if (!m_metalLayer) return;
        
        // Get the framebuffer size (which accounts for Retina scaling)
        // We need to get this from the actual window
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
        
        CGSize newSize = CGSizeMake(framebufferWidth, framebufferHeight);
        m_metalLayer.drawableSize = newSize;
    }
};

// Static factory method implementation
std::unique_ptr<EditorApp> EditorApp::create() {
    return std::make_unique<EditorAppMacOS>();
}

} // namespace Editor
} // namespace FlowGraph

#endif // __APPLE__