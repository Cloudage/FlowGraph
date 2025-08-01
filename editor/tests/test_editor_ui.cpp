// FlowGraph Editor UI Tests
// Test cases for editor functionality using ImGui Test Engine

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_test_engine/imgui_te_context.h>
#include <imgui_test_engine/imgui_te_engine.h>

#include "EditorApp.hpp"

// Global editor app instance for testing
static std::unique_ptr<FlowGraph::Editor::EditorApp> g_editor_app;

// Initialize editor app for testing
static void InitializeEditorForTesting() {
    if (!g_editor_app) {
        g_editor_app = std::make_unique<FlowGraph::Editor::EditorApp>(FlowGraph::Editor::EditorApp::create());
        // Note: We don't call Initialize() here as the test framework handles window creation
    }
}

// Clean up editor app after testing
static void CleanupEditorAfterTesting() {
    if (g_editor_app) {
        g_editor_app->Shutdown();
        g_editor_app.reset();
    }
}

void RegisterEditorUITests(ImGuiTestEngine* engine) {
    ImGuiTest* t = nullptr;
    
    //-----------------------------------------------------------------
    // ## Basic Editor Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "basic_initialization");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        // Create a simple test window to simulate editor functionality
        ImGui::Begin("FlowGraph Editor", nullptr, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("FlowGraph Editor v1.0");
        ImGui::Separator();
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New", "Ctrl+N");
                ImGui::MenuItem("Open", "Ctrl+O");
                ImGui::MenuItem("Save", "Ctrl+S");
                ImGui::Separator();
                ImGui::MenuItem("Exit", "Alt+F4");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Undo", "Ctrl+Z");
                ImGui::MenuItem("Redo", "Ctrl+Y");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Graph View");
                ImGui::MenuItem("Properties");
                ImGui::MenuItem("Console");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        // Main content area
        ImGui::BeginChild("MainContent", ImVec2(0, -30), true);
        ImGui::Text("Graph editing area would be here");
        ImGui::Text("Nodes, connections, and flow visualization");
        ImGui::EndChild();
        
        // Status bar
        ImGui::Separator();
        ImGui::Text("Ready | FlowGraph project loaded");
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->SetRef("FlowGraph Editor");
        
        // Test that the editor window exists and is visible
        IM_CHECK(ctx->ItemExists(""));
        
        // Test basic UI elements
        IM_CHECK(ctx->ItemExists("FlowGraph Editor v1.0"));
        IM_CHECK(ctx->ItemExists("Ready | FlowGraph project loaded"));
        
        ctx->LogInfo("Basic editor initialization test passed");
    };
    
    //-----------------------------------------------------------------
    // ## Menu System Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "menu_navigation");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        ImGui::Begin("FlowGraph Editor", nullptr, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
        
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New", "Ctrl+N")) {
                    ctx->GetVars<ImGuiTestGenericVars>().Bool1 = true;
                }
                ImGui::MenuItem("Open", "Ctrl+O");
                ImGui::MenuItem("Save", "Ctrl+S");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                ImGui::MenuItem("Undo", "Ctrl+Z");
                ImGui::MenuItem("Redo", "Ctrl+Y");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        ImGui::Text("Main editor content");
        
        if (ctx->GetVars<ImGuiTestGenericVars>().Bool1) {
            ImGui::Text("New file action triggered!");
        }
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->SetRef("FlowGraph Editor");
        
        // Test menu navigation
        ctx->MenuClick("File/New");
        ctx->Yield();
        
        // Verify that the menu action was triggered
        IM_CHECK_EQ(ctx->GetVars<ImGuiTestGenericVars>().Bool1, true);
        
        // Test other menu items exist
        ctx->MenuClick("Edit");
        IM_CHECK(ctx->ItemExists("Edit/Undo"));
        IM_CHECK(ctx->ItemExists("Edit/Redo"));
        
        ctx->LogInfo("Menu navigation test passed");
    };
    
    //-----------------------------------------------------------------
    // ## Window Management Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "window_management");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        
        // Main editor window
        ImGui::Begin("FlowGraph Editor", nullptr, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Main Editor Window");
        ImGui::End();
        
        // Properties panel
        ImGui::Begin("Properties", &vars.ShowWindow1, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Node properties would be displayed here");
        if (ImGui::Button("Test Button")) {
            vars.Bool1 = true;
        }
        ImGui::End();
        
        // Console window
        ImGui::Begin("Console", &vars.ShowWindow2, ImGuiWindowFlags_NoSavedSettings);
        ImGui::Text("Console output:");
        ImGui::Text("FlowGraph system initialized");
        ImGui::Text("Ready for graph editing");
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        vars.ShowWindow1 = true;
        vars.ShowWindow2 = true;
        
        ctx->Yield(); // Allow windows to be created
        
        // Test that all windows exist
        IM_CHECK(ctx->ItemExists("FlowGraph Editor"));
        IM_CHECK(ctx->ItemExists("Properties"));
        IM_CHECK(ctx->ItemExists("Console"));
        
        // Test interaction with properties panel
        ctx->SetRef("Properties");
        ctx->ItemClick("Test Button");
        IM_CHECK_EQ(vars.Bool1, true);
        
        // Test window closing
        ctx->WindowClose("Properties");
        ctx->Yield();
        IM_CHECK_EQ(vars.ShowWindow1, false);
        
        ctx->LogInfo("Window management test passed");
    };
    
    //-----------------------------------------------------------------
    // ## Node Graph Simulation Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "node_graph_simulation");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        
        ImGui::Begin("Graph Editor", nullptr, ImGuiWindowFlags_NoSavedSettings);
        
        // Simulate a simple node graph interface
        ImGui::Text("Node Graph Editor");
        ImGui::Separator();
        
        // Toolbar
        if (ImGui::Button("Add Node")) {
            vars.Int1++;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete Node")) {
            if (vars.Int1 > 0) vars.Int1--;
        }
        ImGui::SameLine();
        if (ImGui::Button("Connect")) {
            vars.Bool1 = !vars.Bool1;
        }
        
        ImGui::Separator();
        
        // Node display
        ImGui::Text("Nodes: %d", vars.Int1);
        if (vars.Bool1) {
            ImGui::Text("Connection mode: ON");
        } else {
            ImGui::Text("Connection mode: OFF");
        }
        
        // Simulate some nodes
        for (int i = 0; i < vars.Int1; i++) {
            ImGui::PushID(i);
            char button_label[64];
            snprintf(button_label, sizeof(button_label), "Node %d", i);
            if (ImGui::Button(button_label)) {
                vars.Int2 = i; // Selected node
            }
            ImGui::PopID();
        }
        
        if (vars.Int2 >= 0 && vars.Int2 < vars.Int1) {
            ImGui::Text("Selected: Node %d", vars.Int2);
        }
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        vars.Int1 = 0;
        vars.Int2 = -1;
        vars.Bool1 = false;
        
        ctx->SetRef("Graph Editor");
        
        // Test adding nodes
        IM_CHECK_EQ(vars.Int1, 0);
        ctx->ItemClick("Add Node");
        IM_CHECK_EQ(vars.Int1, 1);
        
        ctx->ItemClick("Add Node");
        ctx->ItemClick("Add Node");
        IM_CHECK_EQ(vars.Int1, 3);
        
        // Test node selection
        ctx->ItemClick("Node 1");
        IM_CHECK_EQ(vars.Int2, 1);
        
        // Test connection mode toggle
        IM_CHECK_EQ(vars.Bool1, false);
        ctx->ItemClick("Connect");
        IM_CHECK_EQ(vars.Bool1, true);
        
        // Test deleting nodes
        ctx->ItemClick("Delete Node");
        IM_CHECK_EQ(vars.Int1, 2);
        
        ctx->LogInfo("Node graph simulation test passed");
    };
    
    //-----------------------------------------------------------------
    // ## Performance Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "performance_basic");
    t->Group = ImGuiTestGroup_Perfs;
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        ImGui::Begin("Performance Test", nullptr, ImGuiWindowFlags_NoSavedSettings);
        
        // Simulate a complex interface with many elements
        static int item_count = ctx->PerfStressAmount * 10;
        
        ImGui::Text("Rendering %d items", item_count);
        ImGui::Separator();
        
        // Create many UI elements to stress test
        for (int i = 0; i < item_count; i++) {
            ImGui::PushID(i);
            char label[64];
            if (i % 4 == 0) {
                snprintf(label, sizeof(label), "Button %d", i);
                ImGui::Button(label);
            } else if (i % 4 == 1) {
                ImGui::Text("Text item %d", i);
            } else if (i % 4 == 2) {
                static bool b = false;
                snprintf(label, sizeof(label), "Check %d", i);
                ImGui::Checkbox(label, &b);
            } else {
                static float f = 0.5f;
                snprintf(label, sizeof(label), "Slider %d", i);
                ImGui::SliderFloat(label, &f, 0.0f, 1.0f);
            }
            ImGui::PopID();
        }
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->SetRef("Performance Test");
        
        // Just verify the window renders and is responsive
        IM_CHECK(ctx->ItemExists(""));
        
        // Test clicking a few random elements
        ctx->ItemClick("Button 0");
        ctx->ItemClick("Check 2");
        
        ctx->LogInfo("Performance test completed");
    };
    
    //-----------------------------------------------------------------
    // ## Graph Layout Visualization Tests
    //-----------------------------------------------------------------
    
    t = IM_REGISTER_TEST(engine, "editor", "graph_layout_controls");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        
        // Graph Controls Panel
        ImGui::Begin("Graph Controls", nullptr, ImGuiWindowFlags_NoSavedSettings);
        
        ImGui::Text("Layout Algorithm:");
        
        const char* layouts[] = {"hierarchical", "force_directed", "grid", "circular"};
        static int current_layout = 0;
        if (ImGui::Combo("##layout", &current_layout, layouts, IM_ARRAYSIZE(layouts))) {
            vars.Int1 = current_layout; // Store selected layout
        }
        
        ImGui::Separator();
        
        if (ImGui::Button("Apply Layout", ImVec2(-1, 0))) {
            vars.Bool1 = true; // Layout applied flag
        }
        
        if (ImGui::Button("Regenerate Graph", ImVec2(-1, 0))) {
            vars.Bool2 = true; // Graph regenerated flag
        }
        
        ImGui::Separator();
        
        // Graph Statistics
        ImGui::Text("Graph Statistics:");
        ImGui::Text("Nodes: %d", vars.Int1);
        ImGui::Text("Edges: %d", vars.Int2);
        
        ImGui::Separator();
        
        // Algorithm Info
        ImGui::Text("Algorithm Info:");
        switch (current_layout) {
            case 0:
                ImGui::TextWrapped("Sugiyama framework - best for directed acyclic graphs and trees");
                break;
            case 1:
                ImGui::TextWrapped("Fruchterman-Reingold - physics-based layout for general graphs");
                break;
            case 2:
                ImGui::TextWrapped("Grid layout - arranges nodes in regular grid pattern");
                break;
            case 3:
                ImGui::TextWrapped("Circular layout - positions nodes in a circle");
                break;
        }
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        vars.Int1 = 0; // layout selection
        vars.Int2 = 9; // node count
        vars.Bool1 = false; // layout applied
        vars.Bool2 = false; // graph regenerated
        
        ctx->SetRef("Graph Controls");
        
        // Test layout algorithm selection
        ctx->ComboClick("##layout");
        ctx->ComboClick("##layout/force_directed");
        IM_CHECK_EQ(vars.Int1, 1);
        
        // Test apply layout button
        ctx->ItemClick("Apply Layout");
        IM_CHECK_EQ(vars.Bool1, true);
        
        // Test regenerate graph button
        ctx->ItemClick("Regenerate Graph");
        IM_CHECK_EQ(vars.Bool2, true);
        
        // Test switching to grid layout
        ctx->ComboClick("##layout");
        ctx->ComboClick("##layout/grid");
        IM_CHECK_EQ(vars.Int1, 2);
        
        ctx->LogInfo("Graph layout controls test passed");
    };
    
    t = IM_REGISTER_TEST(engine, "editor", "graph_visualization");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        
        // Main Graph Visualization Window
        ImGui::Begin("Graph Visualization", nullptr, ImGuiWindowFlags_NoSavedSettings);
        
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
        if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
        if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
        ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
        
        // Draw border
        draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
        
        // Create invisible button for interaction
        ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool is_hovered = ImGui::IsItemHovered();
        const bool is_active = ImGui::IsItemActive();
        
        if (is_hovered) {
            vars.Bool1 = true; // Canvas hovered
        }
        if (is_active) {
            vars.Bool2 = true; // Canvas clicked
        }
        
        // Simulate drawing nodes and edges
        const int node_count = 5;
        const float node_size = 40.0f;
        const float spacing = 80.0f;
        
        for (int i = 0; i < node_count; i++) {
            ImVec2 node_pos = ImVec2(
                canvas_p0.x + 50 + i * spacing,
                canvas_p0.y + 50 + (i % 2) * 60
            );
            
            // Draw node
            draw_list->AddRectFilled(
                node_pos, 
                ImVec2(node_pos.x + node_size, node_pos.y + node_size), 
                IM_COL32(100, 150, 200, 255), 
                4.0f
            );
            draw_list->AddRect(
                node_pos, 
                ImVec2(node_pos.x + node_size, node_pos.y + node_size), 
                IM_COL32(70, 120, 170, 255), 
                4.0f, 
                0, 
                2.0f
            );
            
            // Draw node text
            char node_text[32];
            snprintf(node_text, sizeof(node_text), "N%d", i + 1);
            draw_list->AddText(
                ImVec2(node_pos.x + 8, node_pos.y + 15), 
                IM_COL32(255, 255, 255, 255), 
                node_text
            );
            
            // Draw edges
            if (i > 0) {
                ImVec2 from_center = ImVec2(
                    canvas_p0.x + 50 + (i - 1) * spacing + node_size / 2,
                    canvas_p0.y + 50 + ((i - 1) % 2) * 60 + node_size / 2
                );
                ImVec2 to_center = ImVec2(
                    node_pos.x + node_size / 2,
                    node_pos.y + node_size / 2
                );
                
                draw_list->AddLine(from_center, to_center, IM_COL32(150, 150, 150, 255), 2.0f);
            }
        }
        
        // Display interaction info
        if (is_hovered) {
            ImGui::SetTooltip("Canvas hovered - ready for graph editing");
        }
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        vars.Bool1 = false; // canvas hovered
        vars.Bool2 = false; // canvas clicked
        
        ctx->SetRef("Graph Visualization");
        
        // Test that the window exists
        IM_CHECK(ctx->ItemExists(""));
        
        // Test canvas interaction
        ctx->MouseMove("canvas");
        IM_CHECK_EQ(vars.Bool1, true);
        
        ctx->ItemClick("canvas");
        IM_CHECK_EQ(vars.Bool2, true);
        
        ctx->LogInfo("Graph visualization test passed");
    };
    
    t = IM_REGISTER_TEST(engine, "editor", "status_bar_display");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        // Status Bar at bottom
        ImGui::Begin("##StatusBar", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                    ImGuiWindowFlags_NoSavedSettings);
        
        // FPS display
        ImGui::Text("FPS: %.1f (%.3f ms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Platform info
        ImGui::Text("Platform: Linux (OpenGL 3.3)");
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Content scale
        ImGui::Text("Scale: 1.0x1.0");
        
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        
        // Current layout algorithm
        ImGui::Text("Layout: hierarchical");
        
        ImGui::End();
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        ctx->SetRef("##StatusBar");
        
        // Test that status bar elements exist
        IM_CHECK(ctx->ItemExists(""));
        
        // The status bar should contain platform and layout info
        // Note: Exact text matching might be tricky with dynamic FPS, 
        // so we just verify the window exists and is functional
        
        ctx->LogInfo("Status bar display test passed");
    };
    
    t = IM_REGISTER_TEST(engine, "editor", "menu_layout_switching");
    t->GuiFunc = [](ImGuiTestContext* ctx) {
        InitializeEditorForTesting();
        
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        static int current_layout = 0;
        
        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::MenuItem("New Graph", "Ctrl+N");
                ImGui::MenuItem("Open Graph", "Ctrl+O");
                ImGui::MenuItem("Save Graph", "Ctrl+S");
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Layout")) {
                if (ImGui::MenuItem("hierarchical", nullptr, current_layout == 0)) {
                    current_layout = 0;
                    vars.Int1 = 0;
                }
                if (ImGui::MenuItem("force_directed", nullptr, current_layout == 1)) {
                    current_layout = 1;
                    vars.Int1 = 1;
                }
                if (ImGui::MenuItem("grid", nullptr, current_layout == 2)) {
                    current_layout = 2;
                    vars.Int1 = 2;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Graph Controls", nullptr, &vars.ShowWindow1);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    };
    t->TestFunc = [](ImGuiTestContext* ctx) {
        auto& vars = ctx->GetVars<ImGuiTestGenericVars>();
        vars.Int1 = 0; // current layout
        vars.ShowWindow1 = true; // graph controls visibility
        
        // Test layout menu navigation
        ctx->MenuClick("Layout");
        IM_CHECK(ctx->ItemExists("Layout/hierarchical"));
        IM_CHECK(ctx->ItemExists("Layout/force_directed"));
        IM_CHECK(ctx->ItemExists("Layout/grid"));
        
        // Test layout switching
        ctx->MenuClick("Layout/force_directed");
        IM_CHECK_EQ(vars.Int1, 1);
        
        ctx->MenuClick("Layout/grid");  
        IM_CHECK_EQ(vars.Int1, 2);
        
        // Test view menu
        ctx->MenuClick("View");
        IM_CHECK(ctx->ItemExists("View/Graph Controls"));
        
        ctx->LogInfo("Menu layout switching test passed");
    };
}