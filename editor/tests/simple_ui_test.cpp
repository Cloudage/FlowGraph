// Simple UI Test - Lightweight test without ImGui Test Engine
// Tests basic UI initialization and functionality

#include <iostream>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

class SimpleUITest {
public:
    bool Initialize();
    int Run();
    void Shutdown();
    
private:
    GLFWwindow* window = nullptr;
    bool test_passed = true;
    int frame_count = 0;
    const int max_frames = 60; // Run for 60 frames
    
    bool TestBasicUI();
    bool TestMenus();
    bool TestWindows();
};

bool SimpleUITest::Initialize() {
    // Setup GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return false;
    }

    // GL 3.3 + GLSL 330
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden for testing

    // Create window
    window = glfwCreateWindow(1280, 720, "UI Test", nullptr, nullptr);
    if (!window) {
        return false;
    }

    glfwMakeContextCurrent(window);
    
    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader" << std::endl;
        return false;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

bool SimpleUITest::TestBasicUI() {
    // Test basic ImGui functionality
    ImGui::Begin("Test Window");
    ImGui::Text("Hello, World!");
    ImGui::Button("Test Button");
    ImGui::End();
    
    // Check that ImGui context is valid
    return ImGui::GetCurrentContext() != nullptr;
}

bool SimpleUITest::TestMenus() {
    // Test menu bar functionality
    bool menu_clicked = false;
    
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                menu_clicked = true;
            }
            ImGui::MenuItem("Open");
            ImGui::MenuItem("Save");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Undo");
            ImGui::MenuItem("Redo");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    
    return true; // Menu rendering is successful if we get here
}

bool SimpleUITest::TestWindows() {
    // Test multiple windows
    static bool show_demo = true;
    static bool show_another = true;
    
    if (show_demo) {
        ImGui::Begin("Demo Window", &show_demo);
        ImGui::Text("This is a demo window");
        if (ImGui::Button("Close Me")) {
            show_demo = false;
        }
        ImGui::End();
    }
    
    if (show_another) {
        ImGui::Begin("Another Window", &show_another);
        ImGui::Text("Hello from another window!");
        ImGui::End();
    }
    
    return true;
}

int SimpleUITest::Run() {
    while (!glfwWindowShouldClose(window) && frame_count < max_frames) {
        frame_count++;
        
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Run our tests
        if (frame_count == 10) { // Test basic UI
            test_passed &= TestBasicUI();
            std::cout << "Basic UI test: " << (test_passed ? "PASS" : "FAIL") << std::endl;
        }
        
        if (frame_count == 20) { // Test menus
            test_passed &= TestMenus();
            std::cout << "Menu test: " << (test_passed ? "PASS" : "FAIL") << std::endl;
        }
        
        if (frame_count == 30) { // Test windows
            test_passed &= TestWindows();
            std::cout << "Window test: " << (test_passed ? "PASS" : "FAIL") << std::endl;
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    std::cout << "Simple UI Test completed: " << (test_passed ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << std::endl;
    return test_passed ? 0 : 1;
}

void SimpleUITest::Shutdown() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, char* argv[]) {
    SimpleUITest test;
    
    if (!test.Initialize()) {
        std::cerr << "Failed to initialize UI test" << std::endl;
        return 1;
    }
    
    int result = test.Run();
    test.Shutdown();
    
    return result;
}