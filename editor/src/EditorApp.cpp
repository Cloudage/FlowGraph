#include "EditorApp.hpp"

// Platform-specific includes
#ifdef __APPLE__
    #include <Metal/Metal.h>
    #include <MetalKit/MetalKit.h>
#elif defined(_WIN32)
    #include <d3d11.h>
    #include <dxgi1_4.h>
    #include <wrl/client.h>
    using Microsoft::WRL::ComPtr;
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
#endif

#include <GLFW/glfw3.h>

#ifdef _WIN32
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
    #define GLFW_EXPOSE_NATIVE_COCOA
    #include <GLFW/glfw3native.h>
#endif
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
#include <vector>
#include <algorithm>

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
    
    // Window size callback to handle resizing
    void WindowSizeCallback(GLFWwindow* window, int width, int height) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app && width > 0 && height > 0) {
            app->HandleWindowResize(width, height);
        }
    }
    
    // Content scale callback to handle DPI changes
    void ContentScaleCallback(GLFWwindow* window, float xscale, float yscale) {
        EditorApp* app = static_cast<EditorApp*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->HandleContentScaleChange(xscale, yscale);
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
    // Use OpenGL 3.3 for Linux
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
    glfwSetWindowSizeCallback(m_window, WindowSizeCallback);
    glfwSetWindowContentScaleCallback(m_window, ContentScaleCallback);

#if !defined(__APPLE__) && !defined(_WIN32)
    // Make OpenGL context current (for Linux only)
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync
#endif

    return true;
}

bool EditorApp::SetupRenderingBackend() {
#ifdef __APPLE__
    // Metal setup for macOS
    // Create Metal device
    m_metalDevice = MTLCreateSystemDefaultDevice();
    if (!m_metalDevice) {
        std::cerr << "Failed to create Metal device" << std::endl;
        return false;
    }
    
    // Create command queue
    m_metalCommandQueue = [(id<MTLDevice>)m_metalDevice newCommandQueue];
    if (!m_metalCommandQueue) {
        std::cerr << "Failed to create Metal command queue" << std::endl;
        return false;
    }
    
    // Create Metal layer for the GLFW window
    NSWindow* nsWindow = glfwGetCocoaWindow(m_window);
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = (id<MTLDevice>)m_metalDevice;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    NSView* contentView = [nsWindow contentView];
    [contentView setWantsLayer:YES];
    [contentView setLayer:metalLayer];
    
    m_metalLayer = metalLayer;
    
    std::cout << "Metal initialized successfully" << std::endl;
    return true;
#elif defined(_WIN32)
    // Initialize DirectX 11 for Windows
    HWND hwnd = glfwGetWin32Window(m_window);
    
    // Get window dimensions
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    
    // Create DXGI factory
    ComPtr<IDXGIFactory2> dxgiFactory;
    HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI factory" << std::endl;
        return false;
    }
    
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
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
        return false;
    }
    
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
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create swap chain" << std::endl;
        return false;
    }
    
    // Create render target view
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        std::cerr << "Failed to get back buffer" << std::endl;
        return false;
    }
    
    hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        std::cerr << "Failed to create render target view" << std::endl;
        return false;
    }
    
    std::cout << "DirectX 11 initialized successfully" << std::endl;
    return true;
#else
    // Initialize GLAD for OpenGL (Linux)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    return true;
#endif
}

bool EditorApp::InitializeImGui() {
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
    
    // Set font scaling for high-DPI displays
    if (xscale > 1.0f || yscale > 1.0f) {
        float scale = std::max(xscale, yscale);
        io.FontGlobalScale = scale;
        
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
    }

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup platform/renderer backends
    ImGui_ImplGlfw_InitForOther(m_window, true);

#ifdef __APPLE__
    // Metal backend
    ImGui_ImplMetal_Init((id<MTLDevice>)m_metalDevice);
#elif defined(_WIN32)
    // DirectX 11 backend for Windows
    ImGui_ImplDX11_Init(m_d3dDevice.Get(), m_d3dContext.Get());
#else
    // OpenGL 3.3 backend for Linux
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
    CAMetalLayer* metalLayer = (CAMetalLayer*)m_metalLayer;
    ImGui_ImplMetal_NewFrame([MTLRenderPassDescriptor renderPassDescriptor]);
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
    
    // Display DPI scale information
    ImGui::Text("Content Scale: %.1fx%.1f", m_contentScaleX, m_contentScaleY);
    
    if (ImGui::Button("Show Demo Window")) {
        show_demo_window = true;
    }
    ImGui::End();

    // Rendering
    ImGui::Render();

#ifdef __APPLE__
    // Metal rendering for macOS
    CAMetalLayer* metalLayer = (CAMetalLayer*)m_metalLayer;
    id<CAMetalDrawable> drawable = [metalLayer nextDrawable];
    
    if (drawable) {
        // Create render pass descriptor
        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.45, 0.55, 0.60, 1.0);
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        // Create command buffer
        id<MTLCommandBuffer> commandBuffer = [(id<MTLCommandQueue>)m_metalCommandQueue commandBuffer];
        
        // Create render encoder
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        
        // Render ImGui
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);
        
        // End encoding
        [renderEncoder endEncoding];
        
        // Present drawable
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
    }
#elif defined(_WIN32)
    // DirectX 11 rendering for Windows
    const float clearColor[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    
    // Present the frame
    m_swapChain->Present(1, 0); // VSync enabled
#else
    // OpenGL rendering for Linux
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

void EditorApp::HandleWindowResize(int width, int height) {
    // Request immediate re-render
    m_shouldRender = true;
    
#ifdef _WIN32
    // Recreate DirectX render target with new size
    if (m_d3dDevice && m_swapChain) {
        RecreateDirectXRenderTarget(width, height);
    }
#endif
    
    // Wake up the event loop for immediate re-render
    glfwPostEmptyEvent();
}

void EditorApp::HandleContentScaleChange(float xscale, float yscale) {
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

#ifdef _WIN32
bool EditorApp::RecreateDirectXRenderTarget(int width, int height) {
    if (!m_swapChain || !m_d3dDevice) {
        return false;
    }
    
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
    
    if (FAILED(hr)) {
        std::cerr << "Failed to resize swap chain buffers" << std::endl;
        return false;
    }
    
    // Recreate render target view with new buffer
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hr)) {
        std::cerr << "Failed to get back buffer after resize" << std::endl;
        return false;
    }
    
    hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) {
        std::cerr << "Failed to create render target view after resize" << std::endl;
        return false;
    }
    
    return true;
}
#endif

void EditorApp::CleanupImGui() {
#ifdef __APPLE__
    ImGui_ImplMetal_Shutdown();
    
    // Cleanup Metal resources
    if (m_metalCommandQueue) {
        [(id)m_metalCommandQueue release];
        m_metalCommandQueue = nullptr;
    }
    if (m_metalDevice) {
        [(id)m_metalDevice release];
        m_metalDevice = nullptr;
    }
    if (m_metalLayer) {
        [(id)m_metalLayer release];
        m_metalLayer = nullptr;
    }
#elif defined(_WIN32)
    ImGui_ImplDX11_Shutdown();
    
    // Cleanup DirectX resources
    m_renderTargetView.Reset();
    m_swapChain.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();
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