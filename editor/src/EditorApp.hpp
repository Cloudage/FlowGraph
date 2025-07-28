#pragma once

#include <memory>

struct GLFWwindow;

#ifdef _WIN32
#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;
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

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
    bool m_shouldRender = true;
    
#ifdef _WIN32
    // DirectX 11-specific members for Windows
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11DeviceContext> m_d3dContext;
    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
#endif
    
    // Window properties
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr const char* WINDOW_TITLE = "FlowGraph Editor";
};

} // namespace Editor
} // namespace FlowGraph