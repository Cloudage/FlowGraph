// Linux-specific implementation for EditorApp
#if !defined(__APPLE__) && !defined(_WIN32)

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include "EditorApp.hpp"

extern "C" bool SetupRenderingBackendImpl(GLFWwindow* window, void** deviceOut, void** contextOut, void** swapChainOut, void** renderTargetOut, void** layerOut) {
    // Initialize GLAD for OpenGL (Linux)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // No specific objects to store for OpenGL
    *deviceOut = nullptr;
    *contextOut = nullptr;
    *swapChainOut = nullptr;
    *renderTargetOut = nullptr;
    *layerOut = nullptr;
    
    return true;
}

extern "C" bool InitializeImGuiImpl(GLFWwindow* window, float contentScaleX, float contentScaleY, void* metalDevice, void* d3dDevice, void* d3dContext) {
    // OpenGL 3.3 backend for Linux
    const char* glsl_version = "#version 330";
    return ImGui_ImplOpenGL3_Init(glsl_version);
}

extern "C" void StartImGuiFrameImpl(void* metalLayer) {
    // OpenGL new frame
    ImGui_ImplOpenGL3_NewFrame();
}

extern "C" void RenderImGuiFrameImpl(void* metalDevice, void* metalCommandQueue, void* metalLayer, void* d3dDevice, void* d3dContext, void* renderTargetView, void* swapChain) {
    // OpenGL rendering for Linux
    // Get window from current context (we'll need to pass it somehow)
    GLFWwindow* window = glfwGetCurrentContext();
    
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.25f, 0.25f, 0.25f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window);
}

extern "C" bool RecreateDirectXRenderTargetImpl(void* swapChain, void* d3dDevice, void** renderTargetView, int width, int height) {
    // Not used on Linux
    return true;
}

extern "C" void CleanupImGuiImpl() {
    ImGui_ImplOpenGL3_Shutdown();
}

extern "C" void CleanupRenderingImpl(void** metalDevice, void** metalCommandQueue, void** metalLayer, void** d3dDevice, void** d3dContext, void** swapChain, void** renderTargetView) {
    // No specific cleanup needed for OpenGL
}

float FlowGraph::Editor::EditorApp::GetStatusBarHeight() const {
    return 25.0f * std::max(m_contentScaleX, m_contentScaleY);
}

#endif // !defined(__APPLE__) && !defined(_WIN32)