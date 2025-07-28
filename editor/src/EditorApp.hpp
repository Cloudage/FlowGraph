#pragma once

#include <memory>

struct GLFWwindow;

#ifdef _WIN32
#include <vulkan/vulkan.h>
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
    // Vulkan-specific members for Windows
    VkInstance m_vkInstance = VK_NULL_HANDLE;
    VkPhysicalDevice m_vkPhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_vkDevice = VK_NULL_HANDLE;
    VkQueue m_vkQueue = VK_NULL_HANDLE;
    VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR m_vkSwapchain = VK_NULL_HANDLE;
    VkRenderPass m_vkRenderPass = VK_NULL_HANDLE;
    VkDescriptorPool m_vkDescriptorPool = VK_NULL_HANDLE;
    uint32_t m_vkQueueFamily = 0;
#endif
    
    // Window properties
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr const char* WINDOW_TITLE = "FlowGraph Editor";
};

} // namespace Editor
} // namespace FlowGraph