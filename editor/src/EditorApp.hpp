#pragma once

#include <memory>
#include <string>
#include <vector>

// Forward declarations for FlowGraph layout
namespace flowgraph {
namespace layout {
    template<typename T> class Graph;
    using GraphF = Graph<double>;
}
}

struct GLFWwindow;

#ifdef _WIN32
#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
#endif

#ifdef __APPLE__
#ifdef __OBJC__
@protocol MTLDevice;
@protocol MTLCommandQueue;
@protocol MTLRenderPassDescriptor;
@class CAMetalLayer;
#else
typedef void* id;
#endif
#endif

namespace FlowGraph {
namespace Editor {

/**
 * @brief Main application class for the FlowGraph Editor
 * 
 * Manages the application lifecycle, window creation, ImGui context,
 * and implements on-demand rendering for optimal performance.
 */
class EditorApp {
public:
    EditorApp();
    ~EditorApp();

    /**
     * @brief Initialize the application
     * @return true if initialization was successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Run the main application loop
     * @return Exit code (0 for success)
     */
    int Run();

    /**
     * @brief Shutdown the application and cleanup resources
     */
    void Shutdown();

    /**
     * @brief Request a render on next frame (for on-demand rendering)
     */
    void RequestRender();

    /**
     * @brief Handle window resize events
     * @param width New window width
     * @param height New window height
     */
    void HandleWindowResize(int width, int height);

    /**
     * @brief Handle content scale changes (for high-DPI support)
     * @param xscale New horizontal content scale
     * @param yscale New vertical content scale
     */
    void HandleContentScaleChange(float xscale, float yscale);

private:
    /**
     * @brief Initialize GLFW and create window
     * @return true if successful, false otherwise
     */
    bool InitializeWindow();

    /**
     * @brief Initialize ImGui context and backend
     * @return true if successful, false otherwise
     */
    bool InitializeImGui();

    /**
     * @brief Setup platform-specific rendering backend
     * @return true if successful, false otherwise
     */
    bool SetupRenderingBackend();

    /**
     * @brief Render one frame
     */
    void RenderFrame();

    /**
     * @brief Check if window should close or if we need to redraw
     * @return true if should continue running, false if should exit
     */
    bool ShouldContinue();

    /**
     * @brief Cleanup ImGui resources
     */
    void CleanupImGui();

    /**
     * @brief Cleanup GLFW resources
     */
    void CleanupWindow();
    
    /**
     * @brief Initialize demo graph data
     */
    void InitializeDemoGraph();
    
    /**
     * @brief Apply selected layout algorithm to the graph
     */
    void ApplyLayout();
    
    /**
     * @brief Render the graph visualization
     */
    void RenderGraph();
    
    /**
     * @brief Render the graph controls UI
     */
    void RenderGraphControls();
    
    /**
     * @brief Render the bottom status bar
     */
    void RenderStatusBar();

#ifdef _WIN32
    /**
     * @brief Recreate DirectX render target for window resize (Windows only)
     * @param width New width
     * @param height New height
     * @return true if successful, false otherwise
     */
    bool RecreateDirectXRenderTarget(int width, int height);
#endif

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
    bool m_shouldRender = true;
    
    // Content scale for high-DPI support
    float m_contentScaleX = 1.0f;
    float m_contentScaleY = 1.0f;
    
    // Graph visualization data
    std::unique_ptr<flowgraph::layout::GraphF> m_demoGraph;
    std::string m_currentLayoutAlgorithm = "hierarchical";
    std::vector<std::string> m_availableLayouts = {"hierarchical", "force_directed", "grid", "circular"};
    bool m_showGraphControls = true;
    
#ifdef _WIN32
    // DirectX 11-specific members for Windows
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11DeviceContext> m_d3dContext;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
#endif

#ifdef __APPLE__
    // Metal-specific members for macOS
    id m_metalDevice = nullptr;
    id m_metalCommandQueue = nullptr;
    id m_metalLayer = nullptr;
#endif
    
    // Window properties
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr const char* WINDOW_TITLE = "FlowGraph Editor";
};

} // namespace Editor
} // namespace FlowGraph