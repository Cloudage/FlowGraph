// macOS-specific implementation for EditorApp
#ifdef __APPLE__

#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <QuartzCore/QuartzCore.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <imgui.h>
#include <imgui_impl_metal.h>

#include <iostream>
#include "EditorApp.hpp"

extern "C" bool SetupRenderingBackendImpl(GLFWwindow* window, void** deviceOut, void** contextOut, void** swapChainOut, void** renderTargetOut, void** layerOut) {
    // Metal setup for macOS
    // Create Metal device
    id<MTLDevice> metalDevice = MTLCreateSystemDefaultDevice();
    if (!metalDevice) {
        std::cerr << "Failed to create Metal device" << std::endl;
        return false;
    }
    
    // Create command queue
    id<MTLCommandQueue> metalCommandQueue = [metalDevice newCommandQueue];
    if (!metalCommandQueue) {
        std::cerr << "Failed to create Metal command queue" << std::endl;
        return false;
    }
    
    // Create Metal layer for the GLFW window
    NSWindow* nsWindow = glfwGetCocoaWindow(window);
    CAMetalLayer* metalLayer = [CAMetalLayer layer];
    metalLayer.device = metalDevice;
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // Get window dimensions for proper drawable size
    int windowWidth, windowHeight;
    int framebufferWidth, framebufferHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    
    // Set the drawable size to match framebuffer (for Retina displays)
    metalLayer.drawableSize = CGSizeMake(framebufferWidth, framebufferHeight);
    
    NSView* contentView = [nsWindow contentView];
    [contentView setWantsLayer:YES];
    [contentView setLayer:metalLayer];
    
    // Store results (retain objects for manual memory management)
    *deviceOut = (__bridge_retained void*)metalDevice;
    *contextOut = (__bridge_retained void*)metalCommandQueue;
    if (swapChainOut) *swapChainOut = nullptr; // Not used on macOS
    if (renderTargetOut) *renderTargetOut = nullptr; // Not used on macOS
    *layerOut = (__bridge_retained void*)metalLayer;
    
    std::cout << "Metal initialized successfully" << std::endl;
    return true;
}

extern "C" bool InitializeImGuiImpl(GLFWwindow* window, float contentScaleX, float contentScaleY, void* metalDevice, void* d3dDevice, void* d3dContext) {
    // Metal backend with proper format specification
    id<MTLDevice> device = (__bridge id<MTLDevice>)metalDevice;
    
    // Initialize ImGui Metal backend with BGRA8Unorm format (standard for CAMetalLayer)
    return ImGui_ImplMetal_Init(device);
}

// Store the current drawable globally for this frame
static id<CAMetalDrawable> g_currentDrawable = nil;

extern "C" void StartImGuiFrameImpl(void* metalLayer) {
    // We need to create a proper render pass descriptor for ImGui_ImplMetal_NewFrame
    // This is critical for creating the render pipeline state with correct sample count
    CAMetalLayer* layer = (__bridge CAMetalLayer*)metalLayer;
    
    if (layer) {
        // Get a drawable and store it for later use in RenderImGuiFrameImpl
        g_currentDrawable = [layer nextDrawable];
        if (g_currentDrawable) {
            MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
            renderPassDescriptor.colorAttachments[0].texture = g_currentDrawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.25, 0.25, 0.25, 1.0);
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            
            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            return;
        }
    }
    
    // Clear the drawable if we couldn't get one
    g_currentDrawable = nil;
    
    // Fallback: call with nil (this might still cause issues)
    ImGui_ImplMetal_NewFrame(nil);
}

extern "C" void RenderImGuiFrameImpl(void* metalDevice, void* metalCommandQueue, void* metalLayer, void* d3dDevice, void* d3dContext, void* renderTargetView, void* swapChain) {
    @autoreleasepool {
        // Use the drawable obtained in StartImGuiFrameImpl
        id<CAMetalDrawable> drawable = g_currentDrawable;
        if (!drawable) {
            return; // Skip frame if no drawable available
        }
        
        // Create command buffer
        id<MTLCommandBuffer> commandBuffer = [(__bridge id<MTLCommandQueue>)metalCommandQueue commandBuffer];
        if (!commandBuffer) {
            return;
        }
        
        // Create render pass descriptor (should match the one used in StartImGuiFrameImpl)
        MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
        renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
        renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.25, 0.25, 0.25, 1.0);
        renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
        
        // Create render encoder 
        id<MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        if (!renderEncoder) {
            return;
        }
        
        // Check if ImGui has valid draw data before rendering
        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData && drawData->Valid && drawData->CmdListsCount > 0) {
            // Render ImGui
            ImGui_ImplMetal_RenderDrawData(drawData, commandBuffer, renderEncoder);
        }
        
        // End encoding
        [renderEncoder endEncoding];
        
        // Present drawable and commit
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
        
        // Clear the global drawable for next frame
        g_currentDrawable = nil;
    }
}

extern "C" bool RecreateDirectXRenderTargetImpl(void* swapChain, void* d3dDevice, void** renderTargetView, int width, int height) {
    // Not used on macOS
    return true;
}

extern "C" void UpdateMetalLayerSizeImpl(void* metalLayer, int width, int height) {
    if (!metalLayer) return;
    
    CAMetalLayer* layer = (__bridge CAMetalLayer*)metalLayer;
    
    // Get the framebuffer size (which accounts for Retina scaling)
    // We need to get this from the actual window, but since we don't have direct access,
    // we'll use the provided dimensions and scale appropriately
    
    // For now, assume 2x scaling on Retina displays (this could be improved)
    NSScreen* screen = [NSScreen mainScreen];
    CGFloat scale = screen.backingScaleFactor;
    
    CGSize newSize = CGSizeMake(width * scale, height * scale);
    layer.drawableSize = newSize;
}

extern "C" void CleanupImGuiImpl() {
    ImGui_ImplMetal_Shutdown();
}

extern "C" void CleanupRenderingImpl(void** metalDevice, void** metalCommandQueue, void** metalLayer, void** d3dDevice, void** d3dContext, void** swapChain, void** renderTargetView) {
    // Cleanup Metal resources - release retained objects
    if (*metalLayer) {
        CFRelease(*metalLayer);
        *metalLayer = nullptr;
    }
    if (*metalCommandQueue) {
        CFRelease(*metalCommandQueue);
        *metalCommandQueue = nullptr;
    }
    if (*metalDevice) {
        CFRelease(*metalDevice);
        *metalDevice = nullptr;
    }
}

float FlowGraph::Editor::EditorApp::GetStatusBarHeight() const {
    return 25.0f;
}

#endif // __APPLE__