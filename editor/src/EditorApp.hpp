#pragma once

#include <memory>
#include <string>
#include <vector>
#include <imgui.h>
#include "../../include/flowgraph_layout/LayoutTypes.hpp"

struct GLFWwindow;

namespace FlowGraph {
namespace Editor {

/**
 * @brief Abstract base class for the FlowGraph Editor
 * 
 * Manages the application lifecycle, window creation, ImGui context,
 * and implements on-demand rendering for optimal performance.
 * Platform-specific implementations are provided by subclasses.
 */
class EditorApp {
public:
    virtual ~EditorApp() = default;

    /**
     * @brief Create platform-specific EditorApp instance
     * @return std::unique_ptr<EditorApp> Platform-specific implementation
     */
    static std::unique_ptr<EditorApp> create();

    /**
     * @brief Initialize the application
     * @return true if initialization was successful, false otherwise
     */
    virtual bool Initialize() = 0;

    /**
     * @brief Run the main application loop
     * @return Exit code (0 for success)
     */
    virtual int Run() = 0;

    /**
     * @brief Shutdown the application and cleanup resources
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Request a render on next frame (for on-demand rendering)
     */
    virtual void RequestRender() = 0;

    /**
     * @brief Handle window resize events
     * @param width New window width
     * @param height New window height
     */
    virtual void HandleWindowResize(int width, int height) = 0;

    /**
     * @brief Handle content scale changes (for high-DPI support)
     * @param xscale New horizontal content scale
     * @param yscale New vertical content scale
     */
    virtual void HandleContentScaleChange(float xscale, float yscale) = 0;

protected:
    // Constructor is protected - only subclasses can create instances
    EditorApp() = default;

    /**
     * @brief Initialize GLFW and create window
     * @return true if successful, false otherwise
     */
    virtual bool InitializeWindow() = 0;

    /**
     * @brief Initialize ImGui context and backend
     * @return true if successful, false otherwise
     */
    virtual bool InitializeImGui() = 0;

    /**
     * @brief Setup platform-specific rendering backend
     * @return true if successful, false otherwise
     */
    virtual bool SetupRenderingBackend() = 0;

    /**
     * @brief Render one frame
     */
    virtual void RenderFrame() = 0;

    /**
     * @brief Check if window should close or if we need to redraw
     * @return true if should continue running, false if should exit
     */
    virtual bool ShouldContinue() = 0;

    /**
     * @brief Cleanup ImGui resources
     */
    virtual void CleanupImGui() = 0;

    /**
     * @brief Cleanup GLFW resources
     */
    virtual void CleanupWindow() = 0;

    /**
     * @brief Get platform-specific status bar height
     * @return Status bar height in pixels
     */
    virtual float GetStatusBarHeight() const = 0;
    
    /**
     * @brief Get platform-specific display text
     * @return Platform identification string
     */
    virtual std::string GetPlatformText() const = 0;

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

    /**
     * @brief Check if mouse is over a port
     * @param mouse_pos Mouse position
     * @param port_pos Port center position
     * @param radius Port radius
     * @return true if mouse is over port
     */
    bool IsMouseOverPort(ImVec2 mouse_pos, ImVec2 port_pos, float radius);

protected:
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
    flowgraph::layout::PointF m_dragOffset;                    // Offset from node center to mouse during drag
    size_t m_connectionSourceId = 0;      // 0 means no connection in progress
    bool m_isCreatingConnection = false;
    flowgraph::layout::PointF m_connectionEndPos;              // Current mouse position during connection creation

    // Canvas state for zoom and pan
    flowgraph::layout::PointF m_canvasOffset;
    float m_canvasZoom = 1.0f;
    ImVec2 m_canvasSize;                    // Size of the canvas area
    ImVec2 m_canvasPos;                     // Top-left position of canvas

    // Node editor settings
    static constexpr float NODE_WIDTH = 80.0f;
    static constexpr float NODE_HEIGHT = 40.0f;
    static constexpr float NODE_PORT_RADIUS = 6.0f;
    static constexpr float CONNECTION_THICKNESS = 2.0f;
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    
    // Next node ID for creating new nodes
    size_t m_nextNodeId = 10;
    
    // Canvas panning state
    flowgraph::layout::PointF m_panStart;

    // Window properties
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr const char* WINDOW_TITLE = "FlowGraph Editor";
};

} // namespace Editor
} // namespace FlowGraph
