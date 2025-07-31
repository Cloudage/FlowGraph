#pragma once

#include <memory>
#include <string>
#include <vector>

// Simple 2D vector for internal use
struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

// Forward declarations for FlowGraph layout
namespace flowgraph {
namespace layout {
    template<typename T> class Graph;
    using GraphF = Graph<double>;
    template<typename T> struct Point;
}
}

struct GLFWwindow;
struct ImVec2;

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
    
    /**
     * @brief Handle node selection and interaction
     * @param node_id ID of the node to check interaction for
     * @param node_min Top-left corner of the node in screen coordinates  
     * @param node_max Bottom-right corner of the node in screen coordinates
     * @return true if node was interacted with, false otherwise
     */
    bool HandleNodeInteraction(size_t node_id, ImVec2 node_min, ImVec2 node_max);
    
    /**
     * @brief Handle connection creation between nodes
     * @param from_node_id Source node ID
     * @param to_node_id Target node ID
     */
    void CreateConnection(size_t from_node_id, size_t to_node_id);
    
    /**
     * @brief Delete a connection between nodes
     * @param from_node_id Source node ID
     * @param to_node_id Target node ID
     */
    void DeleteConnection(size_t from_node_id, size_t to_node_id);
    
    /**
     * @brief Create a new node at the specified position
     * @param position Position in graph coordinates
     * @return ID of the created node
     */
    size_t CreateNode(const flowgraph::layout::Point<double>& position);
    
    /**
     * @brief Delete a node by ID
     * @param node_id ID of the node to delete
     */
    void DeleteNode(size_t node_id);
    
    /**
     * @brief Convert screen coordinates to graph coordinates
     * @param screen_pos Screen position
     * @return Graph coordinates
     */
    flowgraph::layout::Point<double> ScreenToGraph(ImVec2 screen_pos);
    
    /**
     * @brief Convert graph coordinates to screen coordinates  
     * @param graph_pos Graph position
     * @return Screen coordinates
     */
    ImVec2 GraphToScreen(const flowgraph::layout::Point<double>& graph_pos);

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
    std::vector<std::string> m_availableLayouts = {"hierarchical", "force_directed", "grid"};
    bool m_showGraphControls = true;
    
    // Node editor state
    size_t m_selectedNodeId = 0;          // 0 means no selection
    bool m_isDraggingNode = false;
    Vec2 m_dragOffset;                    // Offset from node center to mouse during drag
    size_t m_connectionSourceId = 0;      // 0 means no connection in progress
    bool m_isCreatingConnection = false;
    Vec2 m_connectionEndPos;              // Current mouse position during connection creation
    
    // Canvas state for zoom and pan
    Vec2 m_canvasOffset = Vec2(0, 0);
    float m_canvasZoom = 1.0f;
    Vec2 m_canvasSize;                    // Size of the canvas area
    Vec2 m_canvasPos;                     // Top-left position of canvas
    
    // Node editor settings
    static constexpr float NODE_WIDTH = 80.0f;
    static constexpr float NODE_HEIGHT = 40.0f;
    static constexpr float NODE_PORT_RADIUS = 6.0f;
    static constexpr float CONNECTION_THICKNESS = 2.0f;
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    
    // Next node ID for creating new nodes
    size_t m_nextNodeId = 10;
    
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