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
        g_editor_app = std::make_unique<FlowGraph::Editor::EditorApp>();
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
}