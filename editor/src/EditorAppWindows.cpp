// Windows-specific implementation for EditorApp
#ifdef _WIN32

#include "EditorApp.hpp"

#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_dx11.h>

#include <iostream>
#include <memory>

// 错误处理宏
#define CHECK_RESULT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "Failed to " << message << std::endl; \
            return false; \
        } \
    } while(0)

namespace FlowGraph {
namespace Editor {

class EditorAppWindows : public EditorApp {
private:
    // DirectX 11-specific members for Windows
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11DeviceContext> m_d3dContext;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    // 窗口尺寸和缩放计算的通用结构
    struct WindowSizeInfo {
        int windowWidth;
        int windowHeight;
        int framebufferWidth;
        int framebufferHeight;
        ImVec2 displaySize;
        ImVec2 framebufferScale;
    };

public:
    EditorAppWindows() = default;
    virtual ~EditorAppWindows() {
        if (m_initialized) {
            Shutdown();
        }
    }

    bool Initialize() override {
        if (m_initialized) {
            return true;
        }

        CHECK_RESULT(InitializeWindow(), "initialize window");

        if (!SetupRenderingBackend()) {
            CleanupWindow();
            return false;
        }

        if (!InitializeImGui()) {
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
        std::cout << "Platform: Windows (DirectX 11)" << std::endl;

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
        
        // Recreate DirectX render target with new size
        if (m_d3dDevice && m_swapChain) {
            RecreateDirectXRenderTarget(width, height);
        }
        
        // Wake up the event loop for immediate re-render
        glfwPostEmptyEvent();
    }

    void HandleContentScaleChange(float xscale, float yscale) override {
        m_contentScaleX = xscale;
        m_contentScaleY = yscale;
        
        // Update ImGui font scaling for new DPI
        ImGuiIO& io = ImGui::GetIO();
        float scale = CalculateMaxScale(xscale, yscale);
        io.FontGlobalScale = scale;
        
        // Update display size and framebuffer scale
        UpdateImGuiDisplayInfo(io);

        // Request re-render to apply new scaling
        RequestRender();
    }

protected:
    bool InitializeWindow() override {
        // Setup GLFW
        glfwSetErrorCallback([](int error, const char* description) {
            std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });
        
        CHECK_RESULT(glfwInit(), "initialize GLFW");

        // Configure GLFW window hints for DirectX
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // Create window
        m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            return false;
        }

        // Set window user pointer for callbacks
        glfwSetWindowUserPointer(m_window, this);
        
        // Set all necessary callbacks for on-demand rendering
        SetupGLFWCallbacks();

        return true;
    }

    bool SetupRenderingBackend() override {
        // Initialize DirectX 11 for Windows
        HWND hwnd = glfwGetWin32Window(m_window);
        
        // Get window dimensions
        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        
        // Create DXGI factory
        ComPtr<IDXGIFactory2> dxgiFactory;
        HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
        CHECK_RESULT(SUCCEEDED(hr), "create DXGI factory");

        // Create D3D11 device and context
        D3D_FEATURE_LEVEL featureLevel;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
        
        hr = D3D11CreateDevice(
            nullptr,                    // Use default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,                    // No software module
            D3D11_CREATE_DEVICE_DEBUG,  // Enable debug layer in debug builds
            featureLevels,
            ARRAYSIZE(featureLevels),
            D3D11_SDK_VERSION,
            &m_d3dDevice,
            &featureLevel,
            &m_d3dContext
        );
        
        CHECK_RESULT(SUCCEEDED(hr), "create D3D11 device");

        // Create swap chain
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = width;
        swapChainDesc.Height = height;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapChainDesc.Flags = 0;
        
        hr = dxgiFactory->CreateSwapChainForHwnd(
            m_d3dDevice.Get(),
            hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &m_swapChain
        );
        
        CHECK_RESULT(SUCCEEDED(hr), "create swap chain");

        // Create render target view
        ComPtr<ID3D11Texture2D> backBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        CHECK_RESULT(SUCCEEDED(hr), "get back buffer");

        hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        CHECK_RESULT(SUCCEEDED(hr), "create render target view");

        std::cout << "DirectX 11 initialized successfully" << std::endl;
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
        float scale = CalculateMaxScale(xscale, yscale);
        io.FontGlobalScale = scale;
        
        // Configure display size for proper DPI handling
        UpdateImGuiDisplayInfo(io);

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup platform/renderer backends
        ImGui_ImplGlfw_InitForOther(m_window, true);
        
        // DirectX 11 backend
        return ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dContext.Get());
    }

    void RenderFrame() override {
        // Poll events
        glfwPollEvents();
        
        // Update display size and framebuffer scale every frame for accurate rendering
        ImGuiIO& io = ImGui::GetIO();
        UpdateImGuiDisplayInfo(io);

        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
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
        
        // DirectX 11 rendering
        const float clearColor[4] = { 0.25f, 0.25f, 0.25f, 1.00f };
        m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
        m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        
        // Present the frame
        m_swapChain->Present(1, 0); // VSync enabled
    }

    bool ShouldContinue() override {
        return !glfwWindowShouldClose(m_window);
    }

    void CleanupImGui() override {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void CleanupWindow() override {
        // Cleanup DirectX resources
        CleanupDirectXResources();

        if (m_window) {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();
    }

    float GetStatusBarHeight() const override {
        // Use the same minimum scale logic for status bar height
        float scale = CalculateMaxScale(m_contentScaleX, m_contentScaleY);
        if (scale < 1.25f) {
            scale = 1.25f;
        }
        return 24.0f * scale;
    }

private:
    // 计算最大缩放值的通用方法
    float CalculateMaxScale(float xscale, float yscale) const {
        return std::max(xscale, yscale);
    }

    // 窗口尺寸和缩放计算的通用方法
    WindowSizeInfo GetWindowSizeInfo() const {
        WindowSizeInfo info;
        glfwGetWindowSize(m_window, &info.windowWidth, &info.windowHeight);
        glfwGetFramebufferSize(m_window, &info.framebufferWidth, &info.framebufferHeight);

        info.displaySize = ImVec2((float)info.windowWidth, (float)info.windowHeight);

        if (info.windowWidth > 0 && info.windowHeight > 0) {
            info.framebufferScale = ImVec2(
                (float)info.framebufferWidth / info.windowWidth,
                (float)info.framebufferHeight / info.windowHeight
            );
        } else {
            info.framebufferScale = ImVec2(1.0f, 1.0f);
        }

        return info;
    }

    // 更新ImGui显示信息的通用方法
    void UpdateImGuiDisplayInfo(ImGuiIO& io) {
        WindowSizeInfo info = GetWindowSizeInfo();
        io.DisplaySize = info.displaySize;
        io.DisplayFramebufferScale = info.framebufferScale;
    }

    // GLFW回调设置的模板方法
    void SetupGLFWCallbacks() {
        // 通用的请求渲染回调
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

        // 特殊的回调需要不同的处理
        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app && width > 0 && height > 0) app->HandleWindowResize(width, height);
        });

        glfwSetWindowContentScaleCallback(m_window, [](GLFWwindow* window, float xscale, float yscale) {
            EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
            if (app) app->HandleContentScaleChange(xscale, yscale);
        });
    }

    // COM对象清理的专用方法
    void CleanupDirectXResources() {
        m_renderTargetView.Reset();
        m_swapChain.Reset();
        m_d3dContext.Reset();
        m_d3dDevice.Reset();
    }

    bool RecreateDirectXRenderTarget(int width, int height) {
        CHECK_RESULT(m_swapChain && m_d3dDevice, "validate DirectX objects for resize");

        // Release existing render target view
        m_renderTargetView.Reset();
        
        // Resize the swap chain buffers
        HRESULT hr = m_swapChain->ResizeBuffers(
            0,                              // Keep existing buffer count
            width,                          // New width
            height,                         // New height  
            DXGI_FORMAT_UNKNOWN,           // Keep existing format
            0                              // No flags
        );
        
        CHECK_RESULT(SUCCEEDED(hr), "resize swap chain buffers");

        // Recreate render target view with new buffer
        ComPtr<ID3D11Texture2D> backBuffer;
        hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        CHECK_RESULT(SUCCEEDED(hr), "get back buffer after resize");

        hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
        CHECK_RESULT(SUCCEEDED(hr), "create render target view after resize");

        return true;
    }
};

// Static factory method implementation
std::unique_ptr<EditorApp> EditorApp::create() {
    return std::make_unique<EditorAppWindows>();
}

} // namespace Editor
} // namespace FlowGraph

#endif // _WIN32

