#include "EditorApp.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "../../include/flowgraph_layout/Layout.hpp"
#include "../../include/flowgraph_layout/HierarchicalLayout.hpp"
#include "../../include/flowgraph_layout/ForceDirectedLayout.hpp"
#include "../../include/flowgraph_layout/GridLayout.hpp"

using namespace flowgraph::layout;

// Helper functions to convert between Point<float> and ImVec2
namespace {
    ImVec2 ToImVec2(const flowgraph::layout::PointF& v) { return ImVec2(v.x, v.y); }
    flowgraph::layout::PointF FromImVec2(const ImVec2& v) { return flowgraph::layout::PointF(v.x, v.y); }
}

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

void EditorApp::InitializeDemoGraph() {
    using namespace flowgraph::layout;
    
    m_demoGraph = std::make_unique<GraphF>();
    
    // Create a demo hierarchical graph structure
    // Add nodes with different types to showcase layout algorithms
    m_demoGraph->addNode(NodeF(1, {100, 50}, {80, 40}));  // Root node
    m_demoGraph->addNode(NodeF(2, {50, 150}, {80, 40}));  // Left child
    m_demoGraph->addNode(NodeF(3, {150, 150}, {80, 40})); // Right child
    m_demoGraph->addNode(NodeF(4, {20, 250}, {80, 40}));  // Left-left child
    m_demoGraph->addNode(NodeF(5, {80, 250}, {80, 40}));  // Left-right child
    m_demoGraph->addNode(NodeF(6, {140, 250}, {80, 40})); // Right-left child
    m_demoGraph->addNode(NodeF(7, {200, 250}, {80, 40})); // Right-right child
    m_demoGraph->addNode(NodeF(8, {260, 150}, {80, 40})); // Additional node
    m_demoGraph->addNode(NodeF(9, {300, 250}, {80, 40})); // Additional leaf
    
    // Add edges to create tree structure with some cross-connections
    m_demoGraph->addEdge({1, 2}); // Root to left
    m_demoGraph->addEdge({1, 3}); // Root to right
    m_demoGraph->addEdge({2, 4}); // Left to left-left
    m_demoGraph->addEdge({2, 5}); // Left to left-right
    m_demoGraph->addEdge({3, 6}); // Right to right-left
    m_demoGraph->addEdge({3, 7}); // Right to right-right
    m_demoGraph->addEdge({1, 8}); // Root to additional
    m_demoGraph->addEdge({8, 9}); // Additional to leaf
    m_demoGraph->addEdge({5, 6}); // Cross connection for complexity
    
    // Apply initial layout
    ApplyLayout();
}

void EditorApp::ApplyLayout() {
    using namespace flowgraph::layout;
    
    if (!m_demoGraph || m_demoGraph->nodeCount() == 0) return;
    
    std::unique_ptr<LayoutAlgorithm<double>> layout;
    
    if (m_currentLayoutAlgorithm == "hierarchical") {
        layout = std::make_unique<HierarchicalLayout<double>>();
    } else if (m_currentLayoutAlgorithm == "force_directed") {
        layout = std::make_unique<ForceDirectedLayout<double>>();
    } else if (m_currentLayoutAlgorithm == "grid") {
        layout = std::make_unique<GridLayout<double>>();
    } else {
        // Default to hierarchical
        layout = std::make_unique<HierarchicalLayout<double>>();
    }
    
    LayoutConfig config;
    config.nodeSpacing = 60.0;
    config.layerSpacing = 80.0;
    config.iterations = 100;
    
    auto result = layout->apply(*m_demoGraph, config);
    
    if (!result.success) {
        std::cerr << "Layout failed: " << result.errorMessage << std::endl;
    }
}

void EditorApp::RenderGraph() {
    using namespace flowgraph::layout;
    
    ImGui::SetNextWindowPos(ImVec2(250, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Node Editor Canvas", nullptr, ImGuiWindowFlags_None)) {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // Store canvas info for coordinate transformations
        m_canvasPos = canvas_p0;
        m_canvasSize = canvas_sz;

        // Draw background grid
        float grid_step = 64.0f * m_canvasZoom;
        if (grid_step > 8.0f) {
            ImU32 grid_color = IM_COL32(200, 200, 200, 40);
            
            float x = fmodf(m_canvasOffset.x, grid_step);
            for (; x < canvas_sz.x; x += grid_step) {
                draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), 
                                 ImVec2(canvas_p0.x + x, canvas_p1.y), grid_color);
            }
            
            float y = fmodf(m_canvasOffset.y, grid_step);
            for (; y < canvas_sz.y; y += grid_step) {
                draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), 
                                 ImVec2(canvas_p1.x, canvas_p0.y + y), grid_color);
            }
        }
        
        // Draw border
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
        
        // Create invisible button for canvas interaction
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered();
        const bool is_active = ImGui::IsItemActive();
        const ImVec2 mouse_pos = ImGui::GetMousePos();
        
        // Handle canvas panning
        static bool is_panning = false;
        
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
            is_panning = true;
            m_panStart = FromImVec2(mouse_pos);
        }
        if (is_panning) {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                ImVec2 delta = ImVec2(mouse_pos.x - m_panStart.x, mouse_pos.y - m_panStart.y);
                m_canvasOffset.x += delta.x;
                m_canvasOffset.y += delta.y;
                m_panStart = FromImVec2(mouse_pos);
                RequestRender();
            } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) {
                is_panning = false;
            }
        }
        
        // Handle canvas zooming
        if (is_hovered && ImGui::GetIO().MouseWheel != 0.0f) {
            float wheel = ImGui::GetIO().MouseWheel;
            float zoom_factor = wheel > 0 ? 1.1f : 0.9f;
            
            // Zoom towards mouse position
            ImVec2 mouse_canvas = ImVec2(mouse_pos.x - canvas_p0.x, mouse_pos.y - canvas_p0.y);
            ImVec2 mouse_world = ImVec2(
                (mouse_canvas.x - m_canvasOffset.x) / m_canvasZoom,
                (mouse_canvas.y - m_canvasOffset.y) / m_canvasZoom
            );
            
            float new_zoom = std::clamp(m_canvasZoom * zoom_factor, MIN_ZOOM, MAX_ZOOM);
            if (new_zoom != m_canvasZoom) {
                m_canvasZoom = new_zoom;
                
                // Adjust offset to keep mouse position fixed
                m_canvasOffset.x = mouse_canvas.x - mouse_world.x * m_canvasZoom;
                m_canvasOffset.y = mouse_canvas.y - mouse_world.y * m_canvasZoom;
                RequestRender();
            }
        }
        
        // Handle right-click context menu for creating nodes
        if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !m_isDraggingNode) {
            ImGui::OpenPopup("canvas_context");
        }
        
        if (ImGui::BeginPopup("canvas_context")) {
            if (ImGui::MenuItem("Create Node")) {
                auto graph_pos = ScreenToGraph(mouse_pos);
                CreateNode(graph_pos);
                RequestRender();
            }
            ImGui::EndPopup();
        }
        
        // Draw graph if we have nodes
        if (m_demoGraph && m_demoGraph->nodeCount() > 0) {
            const auto& nodes = m_demoGraph->getNodes();
            const auto& edges = m_demoGraph->getEdges();
            
            // Draw edges first (behind nodes)
            for (const auto& edge : edges) {
                auto from_it = nodes.find(edge.from);
                auto to_it = nodes.find(edge.to);
                
                if (from_it != nodes.end() && to_it != nodes.end()) {
                    const auto& from_node = from_it->second;
                    const auto& to_node = to_it->second;
                    
                    ImVec2 from_screen = GraphToScreen(flowgraph::layout::Point<double>(
                        from_node.position.x + from_node.size.x / 2,
                        from_node.position.y + from_node.size.y / 2
                    ));
                    ImVec2 to_screen = GraphToScreen(flowgraph::layout::Point<double>(
                        to_node.position.x + to_node.size.x / 2,
                        to_node.position.y + to_node.size.y / 2
                    ));
                    
                    // Draw connection line
                    draw_list->AddLine(from_screen, to_screen, IM_COL32(150, 150, 150, 255), CONNECTION_THICKNESS);
                    
                    // Draw arrow head
                    ImVec2 direction = ImVec2(to_screen.x - from_screen.x, to_screen.y - from_screen.y);
                    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                    if (length > 0) {
                        direction.x /= length;
                        direction.y /= length;
                        
                        float arrow_size = 8.0f * m_canvasZoom;
                        ImVec2 arrow_tip = ImVec2(
                            to_screen.x - direction.x * NODE_PORT_RADIUS * m_canvasZoom,
                            to_screen.y - direction.y * NODE_PORT_RADIUS * m_canvasZoom
                        );
                        ImVec2 arrow_left = ImVec2(
                            arrow_tip.x + direction.y * arrow_size - direction.x * arrow_size,
                            arrow_tip.y - direction.x * arrow_size - direction.y * arrow_size
                        );
                        ImVec2 arrow_right = ImVec2(
                            arrow_tip.x - direction.y * arrow_size - direction.x * arrow_size,
                            arrow_tip.y + direction.x * arrow_size - direction.y * arrow_size
                        );
                        
                        draw_list->AddTriangleFilled(arrow_tip, arrow_left, arrow_right, IM_COL32(150, 150, 150, 255));
                    }
                }
            }
            
            // Draw connection being created
            if (m_isCreatingConnection && m_connectionSourceId != 0) {
                auto source_it = nodes.find(m_connectionSourceId);
                if (source_it != nodes.end()) {
                    const auto& source_node = source_it->second;
                    ImVec2 source_screen = GraphToScreen(flowgraph::layout::Point<double>(
                        source_node.position.x + source_node.size.x / 2,
                        source_node.position.y + source_node.size.y / 2
                    ));
                    
                    draw_list->AddLine(source_screen, ToImVec2(m_connectionEndPos), IM_COL32(255, 255, 0, 255), CONNECTION_THICKNESS);
                }
            }
            
            // Draw nodes on top
            for (const auto& pair : nodes) {
                const auto& node = pair.second;
                
                ImVec2 node_min = GraphToScreen(node.position);
                ImVec2 node_max = GraphToScreen(flowgraph::layout::Point<double>(
                    node.position.x + node.size.x,
                    node.position.y + node.size.y
                ));
                
                // Check if node is currently being dragged
                bool is_selected = (m_selectedNodeId == node.id);
                
                // Handle node interaction
                if (HandleNodeInteraction(node.id, node_min, node_max)) {
                    RequestRender();
                }
                
                // Draw node background
                ImU32 node_color = is_selected ? IM_COL32(120, 180, 220, 255) : IM_COL32(100, 150, 200, 255);
                ImU32 border_color = is_selected ? IM_COL32(90, 150, 190, 255) : IM_COL32(70, 120, 170, 255);
                
                draw_list->AddRectFilled(node_min, node_max, node_color, 4.0f * m_canvasZoom);
                draw_list->AddRect(node_min, node_max, border_color, 4.0f * m_canvasZoom, 0, 2.0f * m_canvasZoom);
                
                // Draw input port (left side)
                ImVec2 input_port = ImVec2(node_min.x, (node_min.y + node_max.y) * 0.5f);
                draw_list->AddCircleFilled(input_port, NODE_PORT_RADIUS * m_canvasZoom, IM_COL32(255, 100, 100, 255));
                
                // Draw output port (right side)
                ImVec2 output_port = ImVec2(node_max.x, (node_min.y + node_max.y) * 0.5f);
                draw_list->AddCircleFilled(output_port, NODE_PORT_RADIUS * m_canvasZoom, IM_COL32(100, 255, 100, 255));
                
                // Draw node ID text (only if zoom is sufficient)
                if (m_canvasZoom > 0.5f) {
                    char node_text[32];
                    snprintf(node_text, sizeof(node_text), "Node %zu", node.id);
                    
                    float font_scale = std::clamp(m_canvasZoom, 0.5f, 2.0f);
                    ImVec2 text_size = ImGui::CalcTextSize(node_text);
                    text_size.x *= font_scale;
                    text_size.y *= font_scale;
                    
                    ImVec2 text_pos(
                        node_min.x + (node_max.x - node_min.x - text_size.x) * 0.5f,
                        node_min.y + (node_max.y - node_min.y - text_size.y) * 0.5f
                    );
                    
                    // Draw text with custom scaling
                    ImFont* font = ImGui::GetFont();
                    draw_list->AddText(font, font->FontSize * font_scale, text_pos, IM_COL32(255, 255, 255, 255), node_text);
                }
            }
        }
    }
    ImGui::End();
}

void EditorApp::RenderGraphControls() {
    ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(230, 400), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Graph Controls", &m_showGraphControls)) {
        ImGui::Text("Layout Algorithm:");
        
        // Layout selection combo
        if (ImGui::BeginCombo("##layout", m_currentLayoutAlgorithm.c_str())) {
            for (const auto& layout : m_availableLayouts) {
                bool is_selected = (layout == m_currentLayoutAlgorithm);
                if (ImGui::Selectable(layout.c_str(), is_selected)) {
                    m_currentLayoutAlgorithm = layout;
                    ApplyLayout();
                    RequestRender();
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Apply Layout", ImVec2(-1, 0))) {
            ApplyLayout();
            RequestRender();
        }
        
        if (ImGui::Button("Regenerate Graph", ImVec2(-1, 0))) {
            InitializeDemoGraph();
            RequestRender();
        }
        
        ImGui::Separator();
        
        // Graph statistics
        ImGui::Text("Graph Statistics:");
        if (m_demoGraph) {
            ImGui::Text("Nodes: %zu", m_demoGraph->nodeCount());
            ImGui::Text("Edges: %zu", m_demoGraph->edgeCount());
        } else {
            ImGui::Text("Nodes: 0");
            ImGui::Text("Edges: 0");
        }
        
        ImGui::Separator();
        
        // Layout algorithm info
        ImGui::Text("Algorithm Info:");
        if (m_currentLayoutAlgorithm == "hierarchical") {
            ImGui::TextWrapped("Sugiyama framework - best for directed acyclic graphs and trees");
        } else if (m_currentLayoutAlgorithm == "force_directed") {
            ImGui::TextWrapped("Fruchterman-Reingold - physics-based layout for general graphs");
        } else if (m_currentLayoutAlgorithm == "grid") {
            ImGui::TextWrapped("Grid layout - arranges nodes in regular grid pattern");
        }
    }
    ImGui::End();
}

void EditorApp::RenderStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    
    // Get platform-specific status bar height
    float statusBarHeight = GetStatusBarHeight();
    
    ImVec2 status_pos = ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - statusBarHeight);
    ImVec2 status_size = ImVec2(viewport->Size.x, statusBarHeight);
    
    ImGui::SetNextWindowPos(status_pos);
    ImGui::SetNextWindowSize(status_size);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                            ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("##StatusBar", nullptr, flags)) {
        // FPS display
        ImGui::Text("FPS: %.1f (%.3f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Platform info - platform-specific text will be in subclasses
        ImGui::Text("Platform: %s", GetPlatformText().c_str());

        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Content scale
        ImGui::Text("Scale: %.1fx%.1f", m_contentScaleX, m_contentScaleY);
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Current layout algorithm
        ImGui::Text("Layout: %s", m_currentLayoutAlgorithm.c_str());
    }
    ImGui::End();
}

bool EditorApp::HandleNodeInteraction(size_t node_id, ImVec2 node_min, ImVec2 node_max) {
    ImVec2 mouse_pos = ImGui::GetMousePos();
    bool mouse_in_node = (mouse_pos.x >= node_min.x && mouse_pos.x <= node_max.x &&
                         mouse_pos.y >= node_min.y && mouse_pos.y <= node_max.y);
    
    // Check for port interactions first
    ImVec2 input_port = ImVec2(node_min.x, (node_min.y + node_max.y) * 0.5f);
    ImVec2 output_port = ImVec2(node_max.x, (node_min.y + node_max.y) * 0.5f);
    
    float port_radius = NODE_PORT_RADIUS * m_canvasZoom;
    bool mouse_on_input = IsMouseOverPort(mouse_pos, input_port, port_radius);
    bool mouse_on_output = IsMouseOverPort(mouse_pos, output_port, port_radius);
    
    // Handle connection creation from output port
    if (mouse_on_output && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        m_isCreatingConnection = true;
        m_connectionSourceId = node_id;
        m_connectionEndPos = FromImVec2(mouse_pos);
        return true;
    }
    
    // Handle connection completion to input port
    if (mouse_on_input && m_isCreatingConnection && m_connectionSourceId != 0 && m_connectionSourceId != node_id) {
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            CreateConnection(m_connectionSourceId, node_id);
            m_isCreatingConnection = false;
            m_connectionSourceId = 0;
            return true;
        }
    }
    
    // Update connection end position if creating connection
    if (m_isCreatingConnection) {
        m_connectionEndPos = FromImVec2(mouse_pos);
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !mouse_on_input) {
            // Cancel connection if released not on a port
            m_isCreatingConnection = false;
            m_connectionSourceId = 0;
        }
        return true;
    }
    
    // Handle node selection and dragging
    if (mouse_in_node) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_selectedNodeId = node_id;
            m_isDraggingNode = true;
            
            // Calculate drag offset from mouse to node's top-left position in graph coordinates
            auto mouse_graph = ScreenToGraph(mouse_pos);
            auto node_graph = ScreenToGraph(node_min);
            auto offset = mouse_graph - node_graph;
            m_dragOffset.x = offset.x;
            m_dragOffset.y = offset.y;
            return true;
        }
        
        // Handle node deletion with right-click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup(("node_context_" + std::to_string(node_id)).c_str());
            return true;
        }
    }
    
    // Handle node context menu
    if (ImGui::BeginPopup(("node_context_" + std::to_string(node_id)).c_str())) {
        if (ImGui::MenuItem("Delete Node")) {
            DeleteNode(node_id);
            if (m_selectedNodeId == node_id) {
                m_selectedNodeId = 0;
            }
            ImGui::CloseCurrentPopup();
            return true;
        }
        ImGui::EndPopup();
    }
    
    return false;
}

void EditorApp::CreateConnection(size_t from_node_id, size_t to_node_id) {
    if (m_demoGraph && from_node_id != to_node_id) {
        // Check if connection already exists
        const auto& edges = m_demoGraph->getEdges();
        for (const auto& edge : edges) {
            if (edge.from == from_node_id && edge.to == to_node_id) {
                return; // Connection already exists
            }
        }
        
        // Add the new edge
        m_demoGraph->addEdge({from_node_id, to_node_id});
    }
}

void EditorApp::DeleteConnection(size_t from_node_id, size_t to_node_id) {
    if (m_demoGraph) {
        // TODO: Implement edge removal - the layout library doesn't have removeEdge
        // For now, we'll rebuild the graph without the edge
        auto& edges = m_demoGraph->getEdges();
        std::vector<flowgraph::layout::Edge> remaining_edges;
        
        for (const auto& edge : edges) {
            if (!(edge.from == from_node_id && edge.to == to_node_id)) {
                remaining_edges.push_back(edge);
            }
        }
        
        // Create new graph with remaining edges
        auto new_graph = std::make_unique<flowgraph::layout::GraphF>();
        const auto& nodes = m_demoGraph->getNodes();
        for (const auto& pair : nodes) {
            new_graph->addNode(pair.second);
        }
        for (const auto& edge : remaining_edges) {
            new_graph->addEdge(edge);
        }
        m_demoGraph = std::move(new_graph);
    }
}

size_t EditorApp::CreateNode(const flowgraph::layout::Point<double>& position) {
    if (m_demoGraph) {
        size_t new_id = m_nextNodeId++;
        flowgraph::layout::NodeF new_node(new_id, position, {NODE_WIDTH, NODE_HEIGHT});
        m_demoGraph->addNode(new_node);
        return new_id;
    }
    return 0;
}

void EditorApp::DeleteNode(size_t node_id) {
    if (m_demoGraph) {
        // TODO: Implement node removal - the layout library doesn't have removeNode
        // For now, we'll rebuild the graph without the node and its edges
        const auto& nodes = m_demoGraph->getNodes();
        const auto& edges = m_demoGraph->getEdges();
        
        // Create new graph without the deleted node
        auto new_graph = std::make_unique<flowgraph::layout::GraphF>();
        
        // Add all nodes except the one being deleted
        for (const auto& pair : nodes) {
            if (pair.first != node_id) {
                new_graph->addNode(pair.second);
            }
        }
        
        // Add all edges that don't involve the deleted node
        for (const auto& edge : edges) {
            if (edge.from != node_id && edge.to != node_id) {
                new_graph->addEdge(edge);
            }
        }
        
        m_demoGraph = std::move(new_graph);
    }
}

flowgraph::layout::Point<double> EditorApp::ScreenToGraph(ImVec2 screen_pos) {
    // Convert screen coordinates to graph coordinates
    ImVec2 canvas_pos = ImVec2(screen_pos.x - m_canvasPos.x, screen_pos.y - m_canvasPos.y);
    return flowgraph::layout::Point<double>(
        (canvas_pos.x - m_canvasOffset.x) / m_canvasZoom,
        (canvas_pos.y - m_canvasOffset.y) / m_canvasZoom
    );
}

ImVec2 EditorApp::GraphToScreen(const flowgraph::layout::Point<double>& graph_pos) {
    // Convert graph coordinates to screen coordinates
    return ImVec2(
        m_canvasPos.x + graph_pos.x * m_canvasZoom + m_canvasOffset.x,
        m_canvasPos.y + graph_pos.y * m_canvasZoom + m_canvasOffset.y
    );
}

bool EditorApp::IsMouseOverPort(ImVec2 mouse_pos, ImVec2 port_pos, float radius) {
    float dx = mouse_pos.x - port_pos.x;
    float dy = mouse_pos.y - port_pos.y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

} // namespace Editor
} // namespace FlowGraph

